#! /usr/bin/env python

import pygst
pygst.require("0.10")
import gst
import pygtk
import gtk

class Main:
    def __init__(self):
        self.pipeline = gst.Pipeline("mypipeline")

        self.audiotestsrc = gst.element_factory_make("audiotestsrc", "sinesrc")
        self.pipeline.add(self.audiotestsrc)

        self.fakesrc = gst.element_factory_make("fakesrc", "fakesrc")
        self.pipeline.add(self.fakesrc)

        self.sink = gst.element_factory_make("gconfaudiosink", "audiosink")
        self.pipeline.add(self.sink)

        self.audiotestsrc.link(self.sink)

        self.pipeline.set_state(gst.STATE_PLAYING)

start=Main()
gtk.main()

