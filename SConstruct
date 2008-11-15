import os
import os.path
import new
from glob import glob

###############################################################
# Utility functions.
###############################################################

# Similar to env.WhereIs, but always searches os.environ.
def find_on_path(filename):
    paths = os.environ.get('PATH')
    if not paths:
        return None
    for p in paths.split(':'):
        path = os.path.abspath(os.path.join(p, filename))
        if os.path.isfile(path):
            return p
    return None

# Run the given gcc binary, and parses its output to work out the gcc
# version.
def determine_gcc_version(gcc_binary):
    stdout = os.popen('%s --version' % gcc_binary)
    gcc_output = stdout.read().split()
    stdout.close()
    grab_next = False
    for token in gcc_output:
        if grab_next:
            return token
        elif token == '(GCC)':
            grab_next = True
    return None

# Check that a given cross-compiler tool exists. If it does, the path is
# added to the build environment, and the given environment variable is
# set to the tool name.
#
# This is used to check for the presence of a working cross-compiler
# toolchain, and to properly set up the environment to do it. See below
# in the configuration section for details.
def CheckTool(context, envname, toolname=None, hostprefix=None):
    toolname = toolname or envname.lower()
    if hostprefix is None:
        hostprefix = '%s-' % context.env['CROSS_COMPILE_HOST']
    toolname = '%s%s' % (hostprefix, toolname)
    context.Message("Checking for %s..." % toolname)
    toolpath = find_on_path(toolname)
    if not toolpath:
        context.Result('not found')
        return False
    else:
        context.Result('ok')
        context.env[envname] = toolname
        context.env.AppendENVPath('PATH', toolpath)
        return True

# Find the correct variant and version of libgcc.a in the cross-compiler
# toolchain.
def CheckLibGcc(context, gccname):
    context.Message("Locating a cross-compiled libgcc...")
    toolpath = find_on_path(gccname)
    if not toolpath:
        context.Result("%s not found" % toolname)
        return False
    gcc_version = determine_gcc_version(gccname)
    if not gcc_version:
        context.Result("Could not determine gcc version")
        return False
    gcc_install_dir = os.path.split(os.path.normpath(toolpath))[0]
    libgcc_path = os.path.join(gcc_install_dir, 'lib', 'gcc',
                               context.env['CROSS_COMPILE_HOST'],
                               gcc_version, 'interwork', 'libgcc.a')
    if not os.path.isfile(libgcc_path):
        context.Result("Not found")
        return False
    context.Result("ok")
    context.env.Append(NXOS_LIBGCC=libgcc_path)
    return True

def CheckDoxygen(context):
    context.Message("Looking for Doxygen...")
    doxypath = find_on_path('doxygen')
    if doxypath:
        context.Result("ok")
        context.env.AppendENVPath('PATH', doxypath)
        context.env['WITH_DOXYGEN'] = True
    else:
        context.Result("not found")
        context.env['WITH_DOXYGEN'] = False

###############################################################
# Tool that installs an application kernel helper.
###############################################################

def appkernel_tool(env):
    def AppKernelVariant(env, kernel_name, variant_name, app_kernel,
                         app_kernel_lds, keep_lds_lines, delete_lds_lines):
        kvariant = '%s_%s' % (kernel_name, variant_name)

        # Build the variant's ld script...
        sed_args = (["-e 's/%s//'" % x for x in keep_lds_lines] +
                    ["-e '/%s/d'" % x for x in delete_lds_lines])
        variant_lds = env.Command(
            kvariant + '.lds', [app_kernel_lds],
            "cat $SOURCES | sed %s > $TARGET" % ' '.join(sed_args))

        # ... Build the ELF kernel using that script...
        variant_kernel_elf = env.Command(
            kvariant + '.elf',
            [env['NXOS_BASEPLATE'], app_kernel, env['NXOS_LIBGCC']],
            '$LINK -o $TARGET -T %s -Os --gc-sections '
            '$SOURCES' % variant_lds[0].path)
        env.Depends(variant_kernel_elf, variant_lds)

        # ... And make a binary image from that.
        variant_kernel = env.Command(
            kvariant + '.bin', [variant_kernel_elf],
            '$OBJCOPY -O binary $SOURCES $TARGET')

    def AppKernel(self, kernel_name, sources, kernelsize='50k',
                  romkernelsize=None, kernelisbuilt=False):
        romkernelsize = romkernelsize or kernelsize
        env = self.Copy()
        env.Append(CPPPATH=['#systems'])

        # Build a .a with all the application kernel code.
        if kernelisbuilt:
            app_kernel = sources
        else:
            app_kernel = []
            for s in sources:
                app_kernel.append(env.Object(s.split('.')[0], s))

        # Generate an ld script with the right kernel size, then derive
        # the samba and rom variants from that.
        app_kernel_lds = env.Command(
            '%s.lds' % kernel_name, ['#systems/appkernel.lds'],
            "cat $SOURCES | sed -e 's/@RAM_KSIZE@/%s/' -e 's/@ROM_KSIZE@/%s/' "
            ">$TARGET" % (kernelsize, romkernelsize))

        # Build the SAM-BA and ROM kernel variants.
        AppKernelVariant(env, kernel_name, 'samba', app_kernel, app_kernel_lds,
                         ['SAMBA_ONLY'], ['ROM_ONLY'])
        AppKernelVariant(env, kernel_name, 'rom', app_kernel, app_kernel_lds,
                         ['ROM_ONLY'], ['SAMBA_ONLY'])
    appkernel_func = new.instancemethod(AppKernel, env, env.__class__)
    env.AppKernel = appkernel_func

###############################################################
# Options that can be provided on the commandline
###############################################################
buildable_systems = [os.path.split(os.path.dirname(x))[1]
                     for x in glob('systems/*/SConscript')]

opts = Options('build_flags.py')
opts.Add(ListOption('appkernels',
                    'List of application kernels to build '
                    '(by default, only the tests kernel is compiled', 'tests',
                    buildable_systems))

Help('''
Type: 'scons appkernels=...' to build kernels.

The application kernel names are the directory names in the systems/
subdirectory. You can specify several application kernels, separated
by commas.  If you specify no app kernel, only the tests kernel is
built.

Examples

 - Build the Marvin application kernel:
     scons appkernels=marvin

 - Build both Marvin and the tests kernel:
     scons appkernels=tests,marvin

 - Build all available systems (may require extra external dependencies):
     scons appkernels=all

 - Build only the baseplate code:
     scons appkernels=none
''')

###############################################################
# Construct and configure a cross-compiler environment
###############################################################
env = Environment(options = opts,
                  tools = ['gcc', 'as', 'gnulink', 'ar',
                           'doxygen', appkernel_tool],
                  toolpath = ['scons_tools'],
                  NXOS_LIBGCC = [], CPPPATH = '#',
                  WITH_DOXYGEN = True)

if not env.GetOption('clean'):
    conf = Configure(env, custom_tests = {'CheckTool': CheckTool,
                                          'CheckLibGcc': CheckLibGcc,
                                          'CheckDoxygen': CheckDoxygen})
    conf.env['CROSS_COMPILE_HOST'] = 'arm-elf'
    if not (conf.CheckTool('CC', 'gcc') and conf.CheckTool('AR') and
            conf.CheckTool('OBJCOPY') and conf.CheckTool('LINK', 'ld') and
            conf.CheckLibGcc(conf.env['CC'])):
        print "Missing or incomplete arm-elf toolchain, cannot continue!"
        Exit(1)
    conf.CheckDoxygen()
    env = conf.Finish()

env.Replace(CCFLAGS= ['-mcpu=arm7tdmi', '-Os', '-Wextra', '-Wall', '-Werror',
                      '-Wno-div-by-zero', '-Wfloat-equal', '-Wshadow',
                      '-Wpointer-arith', '-Wbad-function-cast',
                      '-Wmissing-prototypes', '-ffreestanding',
                      '-fsigned-char', '-ffunction-sections',
                      '-fdata-sections', '-fomit-frame-pointer',
                      '-msoft-float', '-mthumb-interwork', '-mthumb'],
            ASFLAGS = ['-Wall', '-Werror', '-Os',
                       '-Wa,-mcpu=arm7tdmi,-mfpu=softfpa,-mthumb-interwork'])

# Build the baseplate, and all selected application kernels.
if env.GetOption('clean'):
    appkernels = buildable_systems
else:
    appkernels = env['appkernels']
systems_to_build = ['systems/%s/SConscript' % x for x in appkernels]
SConscript(['base/SConscript'] + systems_to_build, 'env CheckTool')
