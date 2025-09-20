project('yt-dlp-gui-gtk', 'cpp',
    version : '0.1',
    default_options : [
        'cpp_std=c++17',
        'buildtype=release'
    ]
)

gtkmm_dep = dependency('gtkmm-4.0', required: true)

exe = executable('yt-dlp-gui-gtk', 'main.cpp',
    dependencies : gtkmm_dep,
    install : true
)
