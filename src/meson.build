sources = [
  'main.c',
  'db.c'
]

config_h = configure_file(
  input: 'config.h.in',
  output: 'config.h',
  configuration: {'version': meson.project_version()}
)

sqlite3 = dependency('sqlite3')

executable(
  'multiclip',
  sources,
  dependencies: [gtk, sqlite3],
  include_directories: include_directories('.'),
  install: true
)
