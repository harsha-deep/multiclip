project('multiclip', 'cpp',
  version: '0.1',
  default_options: ['cpp_std=c++17']
)

gtkmm_dep = dependency('gtkmm-3.0')

executable(
  'multiclip',
  'src/main.cpp',
  dependencies: gtkmm_dep,
  install: true
)