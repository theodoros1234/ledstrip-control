#!/bin/python
import sys
from Queue import Queue
from ctypes import POINTER, c_ubyte, c_void_p, c_ulong, cast
import os
import time
import math

# From https://github.com/Valodim/python-pulseaudio
from pulseaudio.lib_pulseaudio import *

SINK_NAME = 'PulseEffects_apps'
#SINK_NAME = 'alsa_output.pci-0000_00_14.2.analog-stereo'  # edit to match your sink
METER_RATE = 60
MAX_IDLE_TIME = 5
MAX_SAMPLE_VALUE = 127
DISPLAY_SCALE = 2
MAX_SPACES = MAX_SAMPLE_VALUE >> DISPLAY_SCALE
prev_sample=0
DEV_PATH = "/dev/ttyACM0"

class PeakMonitor(object):

    def __init__(self, sink_name, rate):
        self.sink_name = sink_name
        self.rate = rate

        # Wrap callback methods in appropriate ctypefunc instances so
        # that the Pulseaudio C API can call them
        self._context_notify_cb = pa_context_notify_cb_t(self.context_notify_cb)
        self._sink_info_cb = pa_sink_info_cb_t(self.sink_info_cb)
        self._stream_read_cb = pa_stream_request_cb_t(self.stream_read_cb)

        # stream_read_cb() puts peak samples into this Queue instance
        self._samples = Queue()

        # Create the mainloop thread and set our context_notify_cb
        # method to be called when there's updates relating to the
        # connection to Pulseaudio
        _mainloop = pa_threaded_mainloop_new()
        _mainloop_api = pa_threaded_mainloop_get_api(_mainloop)
        context = pa_context_new(_mainloop_api, 'LED_strip_audio_client')
        pa_context_set_state_callback(context, self._context_notify_cb, None)
        pa_context_connect(context, None, 0, None)
        pa_threaded_mainloop_start(_mainloop)

    def __iter__(self):
        while True:
            yield self._samples.get()

    def context_notify_cb(self, context, _):
        state = pa_context_get_state(context)

        if state == PA_CONTEXT_READY:
            print "Pulseaudio connection ready..."
            # Connected to Pulseaudio. Now request that sink_info_cb
            # be called with information about the available sinks.
            o = pa_context_get_sink_info_list(context, self._sink_info_cb, None)
            pa_operation_unref(o)

        elif state == PA_CONTEXT_FAILED :
            print "Connection failed"

        elif state == PA_CONTEXT_TERMINATED:
            print "Connection terminated"

    def sink_info_cb(self, context, sink_info_p, _, __):
        if not sink_info_p:
            return

        sink_info = sink_info_p.contents
        print '-'* 60
        print 'index:', sink_info.index
        print 'name:', sink_info.name
        print 'description:', sink_info.description

        if sink_info.name == self.sink_name:
            # Found the sink we want to monitor for peak levels.
            # Tell PA to call stream_read_cb with peak samples.
            print
            print 'setting up peak recording using', sink_info.monitor_source_name
            print
            samplespec = pa_sample_spec()
            samplespec.channels = 1
            samplespec.format = PA_SAMPLE_S16LE
            samplespec.rate = self.rate

            pa_stream = pa_stream_new(context, "peak detect demo", samplespec, None)
            pa_stream_set_read_callback(pa_stream,
                                        self._stream_read_cb,
                                        sink_info.index)
            pa_stream_connect_record(pa_stream,
                                     sink_info.monitor_source_name,
                                     None,
                                     PA_STREAM_PEAK_DETECT)

    def stream_read_cb(self, stream, length, index_incr):
        data = c_void_p()
        pa_stream_peek(stream, data, c_ulong(length))
        data = cast(data, POINTER(c_ubyte))
        for i in xrange(length):
            # When PA_SAMPLE_U8 is used, samples values range from 128
            # to 255 because the underlying audio data is signed but
            # it doesn't make sense to return signed peaks.
            self._samples.put(data[i] - 128)
        pa_stream_drop(stream)

def main():
    print '\033[?1049h\033[?25l\033[2J\033[0;0H'
    try:
      monitor = PeakMonitor(SINK_NAME, METER_RATE)
      os.system("stty -F "+DEV_PATH+""" -hup || sleep 1
      stty -F """+DEV_PATH+" ospeed 115200 || sleep 1""")
      f=open(DEV_PATH,"w")
      f.write("\$H")
      f.flush()
      prev_sample=0
      prev_sample_raw=0
      gain=30.0
      idle_time=0
      print '\033[s'
      for sample in monitor:
          # Skip samples if running behind
          if monitor._samples.qsize()>20:
            monitor._samples.queue.clear()
            print "\033[uSkipping samples\n\033[s"
            continue

          # Scale to 0-1
          sample+=128.0
          sample/=255.0
          
          # Apply & adjust gain
          sample*=gain
          if sample>1.0:
            gain/=sample
            sample=1.0
          elif gain<30.0:
            gain*=1.0003
          sample=math.pow(sample,2)*255
          prev_sample_raw=sample

          # Decay
          if sample<(prev_sample*0.95):
            sample=prev_sample*0.95
          prev_sample=sample

          # Scale to 0-255 and convert to integer
          sample=int(round(sample))
          if sample>255:
            sample=255
          
          # Count idle time
          if sample==0:
            idle_time+=1
          else:
            idle_time=0

          # Send info to Arduino
          f.write("#");
          # Use max brightness if idle
          if idle_time//METER_RATE>=MAX_IDLE_TIME*2:
            f.write("FF")
          else:
          # Otherwise, use sample controlled brightness
            if sample<16:
              f.write("0")
            f.write(hex(sample).split('x')[-1])
          f.flush()
          
          # Display info on terminal
          terminal_size=os.popen('stty size','r').read().split()
          bar_width_max=int(terminal_size[1])-16
          bar_width=sample*bar_width_max/255
          bar='\033[92m'
          if bar_width<bar_width_max/2:
            bar+='>'*bar_width
          else:
            bar+='>'*(bar_width_max/2)+'\033[93m'
            bar_width-=bar_width_max/2
            if bar_width<bar_width_max/3:
              bar+='>'*bar_width
            else:
              bar+='>'*(bar_width_max/3)+'\033[91m'
              bar_width-=bar_width_max/3
              bar+='>'*bar_width
          print '\033[2K\033[%s;0H(%+6.2fdB) %3d %s\033[0m' % (terminal_size[0], 10*math.log(gain,10), sample, bar),
          
          #print monitor._samples.qsize() , pa_stream_get_sample_spec(pa_stream).rate , '    \r'
          sys.stdout.flush()
          
          #print sample
    except:
      print "\033[?25h\033[?1049l"
      f.write("$S#FF");
      f.close()

if __name__ == '__main__':
    main()
