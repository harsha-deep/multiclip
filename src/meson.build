sources = [
  'main.c'
]

config_h = configure_file(
  input: 'config.h.in',
  output: 'config.h',
  configuration: {'version': meson.project_version()}
)

executable(
  'multiclip',
  sources,
  dependencies: [gtk],
  include_directories: include_directories('.'),
  install: true
)
