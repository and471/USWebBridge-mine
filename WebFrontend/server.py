#!/usr/bin/env python3

import os
import sass
from http.server import SimpleHTTPRequestHandler, HTTPServer

# HTTPRequestHandler class
class SaSSRequestHandler(SimpleHTTPRequestHandler):

  # GET
  def do_GET(self):

    path = self.translate_path(self.path)
    scss_path = path.replace(".css", ".scss")


    if (path.endswith(".css") and not os.path.exists(path) and os.path.exists(scss_path)):
        with open(path, "w") as css:
         css.write(sass.compile(filename=scss_path))
        super().do_GET()
        os.remove(path)
    else:
        super().do_GET()


def run():
  server_address = ('0.0.0.0', 8000)
  httpd = HTTPServer(server_address, SaSSRequestHandler)
  httpd.serve_forever()

run()