sources = ['main.c', 'db.c', 'tray.c']

config_h = configure_file(
  input: 'config.h.in',
  output: 'config.h',
  configuration: {'version': meson.project_version()}
)

sqlite3 = dependency('sqlite3')
x11     = dependency('x11', required: false)

extra_deps = []
if x11.found()
  extra_deps += x11
  message('X11 found — Super+V hotkey enabled')
else
  message('X11 not found — Super+V hotkey disabled')
endif

executable(
  'multiclip',
  sources,
  dependencies: [gtk, sqlite3] + extra_deps,
  include_directories: include_directories('.'),
  install: true
)