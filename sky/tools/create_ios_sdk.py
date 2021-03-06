#!/usr/bin/env python
# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import subprocess
import shutil
import sys
import os


def main():
  parser = argparse.ArgumentParser(description='Creates the Flutter iOS SDK')

  parser.add_argument('--dst', type=str, required=True)
  parser.add_argument('--device-out-dir', type=str, required=True)
  parser.add_argument('--simulator-out-dir', type=str, required=True)

  args = parser.parse_args()

  device_sdk = os.path.join(args.device_out_dir, 'Flutter')
  simulator_sdk = os.path.join(args.simulator_out_dir, 'Flutter')

  flutter_framework_binary = 'Flutter.framework/Flutter'

  device_dylib = os.path.join(args.device_out_dir,
      flutter_framework_binary)
  simulator_dylib = os.path.join(args.simulator_out_dir,
      flutter_framework_binary)

  if not os.path.isdir(device_sdk):
    print 'Cannot find iOS device SDK at', device_sdk
    return 1

  if not os.path.isdir(simulator_sdk):
    print 'Cannot find iOS simulator SDK at', simulator_sdk
    return 1

  if not os.path.isfile(device_dylib):
    print 'Cannot find iOS device dylib at', device_dylib
    return 1

  if not os.path.isfile(simulator_dylib):
    print 'Cannot find iOS device dylib at', simulator_dylib
    return 1

  shutil.rmtree(args.dst, True)
  shutil.copytree(device_sdk, args.dst)

  sim_tools = 'Tools/iphonesimulator'
  shutil.copytree(os.path.join(simulator_sdk, sim_tools),
                  os.path.join(args.dst, sim_tools))

  subprocess.call([
    'lipo',
    device_dylib,
    simulator_dylib,
    '-create',
    '-output',
    os.path.join(args.dst, 'Tools/common/Flutter.framework/Flutter')
  ])


if __name__ == '__main__':
  sys.exit(main())
