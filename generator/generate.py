#!/usr/bin/env python

from __future__ import division

import sys
import os
import re
import shutil
import filecmp
from collections import namedtuple, OrderedDict
from itertools import groupby
import operator
import json


from jinja2 import Environment, PackageLoader, select_autoescape
env = Environment(
    loader=PackageLoader('templates', ''),
    autoescape=select_autoescape([]),
    keep_trailing_newline=True,
    lstrip_blocks=True
)

gen_dir = os.path.dirname(os.path.abspath(__file__))
json_dir = os.path.join(gen_dir, "json")
dest_dir = os.path.abspath(os.path.join(gen_dir, "..", "src", "generated"))
tmp_dir = os.path.join(dest_dir, "tmp")

# Additional Jinja 2 functions
def widget(field):
    t = field["type"]
    if t == "number":
        return "QSpinBox"
    elif t == "text":
        return "QLineEdit"
    return "QWidget"
# End of Jinja 2 functions

def get_events(filename='events.json'):
    ev = json.load(open(os.path.join(json_dir, filename)))
    for e in ev:
        filename = re.sub("([A-Z])", r"_\1", e["name"]).lower()[1:] + "_widget"
        e["filename"] = filename
        if "args" not in e:
            e["args"] = []
        for i, arg in enumerate(e["args"]):
            arg["index"] = i
        e["allargs"] = e["args"]
        if "string" in e:
            e["string"]["type"] = "string"
            e["string"]["field"] = "text"
            e["allargs"] = [e["string"]] + e["allargs"]
    return ev

def openToRender(path):
    subdir = os.path.dirname(path)
    if not os.path.exists(subdir):
        os.makedirs(subdir)
    return open(path, 'w')

def generate():
    if not os.path.exists(tmp_dir):
        os.mkdir(tmp_dir)

    for event in events:
        filepath = os.path.join(tmp_dir, event["filename"])

        with openToRender(filepath + ".cpp") as f:
            f.write(event_source_tmpl.render(**event))
            
        with openToRender(filepath + ".h") as f:
            f.write(event_header_tmpl.render(**event))
            
        with openToRender(filepath + ".ui") as f:
            f.write(event_ui_tmpl.render(**event))

    for dirname, subdirlist, filelist in os.walk(tmp_dir, topdown=False):
        subdir = os.path.relpath(dirname, tmp_dir)

        for tmp_file in filelist:
            tmp_path = os.path.join(tmp_dir, subdir, tmp_file)
            dest_path = os.path.join(dest_dir, subdir, tmp_file)
            dest_subdir = os.path.dirname(dest_path)
            if not os.path.exists(dest_subdir):
                os.mkdir(dest_subdir)
            if not (os.path.exists(dest_path) and filecmp.cmp(tmp_path, dest_path)):
                shutil.copyfile(tmp_path, dest_path)
            os.remove(tmp_path)
        os.rmdir(os.path.join(dirname))

def main(argv):
    if not os.path.exists(dest_dir):
        os.mkdir(dest_dir)

    global event_header_tmpl, event_source_tmpl, event_ui_tmpl
    global events

    events = get_events('events.json')

    # Setup Jinja
    env.filters["widget"] = widget
    #env.tests['needs_ctor'] = needs_ctor

    globals = dict(
        events=events
    )

    event_header_tmpl = env.get_template('event_header.tmpl', globals=globals)
    event_source_tmpl = env.get_template('event_source.tmpl', globals=globals)
    event_ui_tmpl = env.get_template('event_ui.tmpl', globals=globals)

    generate()

if __name__ == '__main__':
    main(sys.argv)
