#!/usr/bin/env python3
Import("env")

print("Current CLI targets", COMMAND_LINE_TARGETS)
print("Current Build targets", BUILD_TARGETS)

def post_program_action(source, target, env):
    print("Program has been built!")
    program_path = target[0].get_abspath()
    print("Program path", program_path)
    # Use case: sign a firmware, do any manipulations with ELF, etc
    # env.Execute(f"sign --elf {program_path}")

env.AddPostAction("$PROGPATH", post_program_action)


def tst(source, target, env):
   print("tst")

# Build file system manually
def build_parts(source, target, env):
  target_fsbin = env.DataToBin(join("$BUILD_DIR", "${ESP32_FS_IMAGE_NAME}"), "$PROJECT_DATA_DIR")
  env.noCache(target_fsbin)
  AlwaysBuild(target_fsbin)

def generate_image(source, target, env):
  print("!!! Building images...")
  build_parts(source, target, env)

# default target is elf file (if not target buildfs or uploadfs on command line)
# so this target will build elf file, then next build fonts and file system
env.AddCustomTarget(
  "firmware",
  "$BUILD_DIR/$(PROGNAME}.elf",
  generate_image)

env.AddCustomTarget(
  "buildfonts",
  "$BUILD_DIR/$(PROGNAME}.elf",
  env.VerboseAction(" ".join([
    "xtensa-esp32-elf-ld", "-T", "fontlink.ld", "--oformat=binary", "-o", "$BUILD_DIR/fonts.bin", "$BUILD_DIR/src/src/fonts/fonts.cpp.o" ]),
    "Building $BUILD_DIR/fonts.bin"))

env.AddCustomTarget(
  "uploadfonts",
  "buildfonts",
  "scripts/uploadfonts.py $BUILD_DIR/fonts.bin $PARTITIONS_TABLE_CSV"
)

env.AddCustomTarget(
  "uploadall",
  "firmware",
  tst)


dict = env.Dictionary()
keys = dict.keys()
for key in keys:
   print ("construction variable = '%s', value = '%s'" % (key, dict[key]))


