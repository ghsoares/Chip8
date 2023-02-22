env = Environment()

sources = Glob('main.cpp') + Glob('src/*.cpp')

env.Append(CXXFLAGS = ['/DEBUG'])
env.Append(CPPPATH = ['thirdparty/freeglut/include/GL'], LIBS = ['freeglut'], LIBPATH = ['thirdparty/freeglut/lib/x64'])
env.Program(target='target/chip8', source=sources)