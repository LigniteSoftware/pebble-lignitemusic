AR = 'arm-none-eabi-ar'
ARFLAGS = 'rcs'
AS = 'arm-none-eabi-gcc'
BINDIR = '/usr/local/bin'
BLOCK_MESSAGE_KEYS = []
BUILD_DIR = 'chalk'
BUILD_TYPE = 'app'
BUNDLE_BIN_DIR = 'chalk'
BUNDLE_NAME = 'music-pebble.pbw'
CC = ['arm-none-eabi-gcc']
CCLNK_SRC_F = []
CCLNK_TGT_F = ['-o']
CC_NAME = 'gcc'
CC_SRC_F = []
CC_TGT_F = ['-c', '-o']
CC_VERSION = ('4', '7', '2')
CFLAGS = ['-std=c99', '-mcpu=cortex-m3', '-mthumb', '-ffunction-sections', '-fdata-sections', '-g', '-fPIE', '-Os', '-D_TIME_H_', '-Wall', '-Wextra', '-Werror', '-Wno-unused-parameter', '-Wno-error=unused-function', '-Wno-error=unused-variable']
CFLAGS_MACBUNDLE = ['-fPIC']
CFLAGS_cshlib = ['-fPIC']
CPPPATH_ST = '-I%s'
DEFINES = ['RELEASE', 'PBL_PLATFORM_CHALK', 'PBL_COLOR', 'PBL_ROUND', 'PBL_MICROPHONE', 'PBL_SMARTSTRAP', 'PBL_HEALTH', 'PBL_SDK_3']
DEFINES_ST = '-D%s'
DEST_BINFMT = 'elf'
DEST_CPU = 'arm'
DEST_OS = 'darwin'
INCLUDES = ['chalk']
LD = 'arm-none-eabi-ld'
LIBDIR = '/usr/local/lib'
LIBPATH_ST = '-L%s'
LIB_DIR = 'node_modules'
LIB_ST = '-l%s'
LINKFLAGS = ['-mcpu=cortex-m3', '-mthumb', '-Wl,--gc-sections', '-Wl,--warn-common', '-fPIE', '-Os']
LINKFLAGS_MACBUNDLE = ['-bundle', '-undefined', 'dynamic_lookup']
LINKFLAGS_cshlib = ['-shared']
LINKFLAGS_cstlib = ['-Wl,-Bstatic']
LINK_CC = ['arm-none-eabi-gcc']
MESSAGE_KEYS = {}
MESSAGE_KEYS_HEADER = '/Users/edwinfinch/Desktop/Lignite Music/music-pebble/build/include/message_keys.auto.h'
PEBBLE_SDK_COMMON = '/Users/edwinfinch/Library/Application Support/Pebble SDK/SDKs/current/sdk-core/pebble/common'
PEBBLE_SDK_PLATFORM = '/Users/edwinfinch/Library/Application Support/Pebble SDK/SDKs/current/sdk-core/pebble/chalk'
PEBBLE_SDK_ROOT = '/Users/edwinfinch/Library/Application Support/Pebble SDK/SDKs/current/sdk-core/pebble'
PLATFORM = {'TAGS': ['chalk', 'color', 'round'], 'ADDITIONAL_TEXT_LINES_FOR_PEBBLE_H': [], 'MAX_APP_BINARY_SIZE': 65536, 'MAX_RESOURCES_SIZE': 1048576, 'MAX_APP_MEMORY_SIZE': 65536, 'MAX_WORKER_MEMORY_SIZE': 10240, 'NAME': 'chalk', 'BUNDLE_BIN_DIR': 'chalk', 'BUILD_DIR': 'chalk', 'MAX_RESOURCES_SIZE_APPSTORE': 262144, 'DEFINES': ['PBL_PLATFORM_CHALK', 'PBL_COLOR', 'PBL_ROUND', 'PBL_MICROPHONE', 'PBL_SMARTSTRAP', 'PBL_HEALTH']}
PLATFORM_NAME = 'chalk'
PREFIX = '/usr/local'
PROJECT_INFO = {'appKeys': {}, u'watchapp': {u'watchface': False}, u'displayName': u'Lignite Music', u'uuid': u'edf76057-f3ef-4de6-b841-cb9532a81a5a', 'messageKeys': {}, 'companyName': u'Edwin Finch', u'enableMultiJS': False, u'sdkVersion': u'3', 'versionLabel': u'0.4', u'targetPlatforms': [u'aplite', u'basalt', u'chalk'], 'longName': u'Lignite Music', 'shortName': u'Lignite Music', u'resources': {u'media': [{u'type': u'png', u'name': u'ICON_VOLUME_UP', u'file': u'images/volume_up.png'}, {u'type': u'png', u'name': u'ICON_VOLUME_DOWN', u'file': u'images/volume_down.png'}, {u'type': u'png', u'name': u'ICON_PLAY', u'file': u'images/play.png'}, {u'type': u'png', u'name': u'ICON_PAUSE', u'file': u'images/pause.png'}, {u'type': u'png', u'name': u'ICON_NEXT_SONG', u'file': u'images/next_song.png'}, {u'type': u'png', u'name': u'ICON_LAST_SONG', u'file': u'images/last_song.png'}, {u'type': u'png', u'name': u'ICON_NOW_PLAYING', u'file': u'images/playing.png'}, {u'type': u'png', u'name': u'ICON_PLAYLISTS', u'file': u'images/playlists.png'}, {u'type': u'png', u'name': u'ICON_ARTISTS', u'file': u'images/artists.png'}, {u'type': u'png', u'name': u'ICON_ALBUMS', u'file': u'images/albums.png'}, {u'type': u'png', u'name': u'ICON_COMPOSERS', u'file': u'images/composers.png'}, {u'type': u'png', u'name': u'ICON_GENRES', u'file': u'images/genres.png'}, {u'type': u'png', u'name': u'NO_ALBUM_ART', u'file': u'images/no_album_art.png'}, {u'type': u'png', u'name': u'ICON_TITLE', u'file': u'images/title.png'}, {u'type': u'png', u'name': u'ICON_MORE', u'file': u'images/more.png'}]}, 'name': u'Lignite Music'}
REQUESTED_PLATFORMS = [u'aplite', u'basalt', u'chalk']
RESOURCES_JSON = [{u'type': u'png', u'name': u'ICON_VOLUME_UP', u'file': u'images/volume_up.png'}, {u'type': u'png', u'name': u'ICON_VOLUME_DOWN', u'file': u'images/volume_down.png'}, {u'type': u'png', u'name': u'ICON_PLAY', u'file': u'images/play.png'}, {u'type': u'png', u'name': u'ICON_PAUSE', u'file': u'images/pause.png'}, {u'type': u'png', u'name': u'ICON_NEXT_SONG', u'file': u'images/next_song.png'}, {u'type': u'png', u'name': u'ICON_LAST_SONG', u'file': u'images/last_song.png'}, {u'type': u'png', u'name': u'ICON_NOW_PLAYING', u'file': u'images/playing.png'}, {u'type': u'png', u'name': u'ICON_PLAYLISTS', u'file': u'images/playlists.png'}, {u'type': u'png', u'name': u'ICON_ARTISTS', u'file': u'images/artists.png'}, {u'type': u'png', u'name': u'ICON_ALBUMS', u'file': u'images/albums.png'}, {u'type': u'png', u'name': u'ICON_COMPOSERS', u'file': u'images/composers.png'}, {u'type': u'png', u'name': u'ICON_GENRES', u'file': u'images/genres.png'}, {u'type': u'png', u'name': u'NO_ALBUM_ART', u'file': u'images/no_album_art.png'}, {u'type': u'png', u'name': u'ICON_TITLE', u'file': u'images/title.png'}, {u'type': u'png', u'name': u'ICON_MORE', u'file': u'images/more.png'}]
RPATH_ST = '-Wl,-rpath,%s'
SDK_VERSION_MAJOR = 5
SDK_VERSION_MINOR = 79
SHLIB_MARKER = None
SIZE = 'arm-none-eabi-size'
SONAME_ST = '-Wl,-h,%s'
STLIBPATH_ST = '-L%s'
STLIB_MARKER = None
STLIB_ST = '-l%s'
SUPPORTED_PLATFORMS = ['aplite', 'basalt', 'chalk']
TARGET_PLATFORMS = ['chalk', 'basalt', 'aplite']
TIMESTAMP = 1471317183
cprogram_PATTERN = '%s'
cshlib_PATTERN = 'lib%s.so'
cstlib_PATTERN = 'lib%s.a'
macbundle_PATTERN = '%s.bundle'
