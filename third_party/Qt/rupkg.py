def process_options(opt, command, ctx):
    if opt.qt_static:
        command.add("Qt_BUILD_STATIC", 'on')

def config_options(parser):
    parser.add_argument("--qt-static", help="Build static Qt", dest="qt_static", action='store_true', default=False)