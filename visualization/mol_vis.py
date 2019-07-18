# Run this bad boy in interative mode

import sys
import argparse

import vispy
from vispy import scene
import numpy as np

COLORS_ENUM = [(0.4, 0.4, 1), (1, 1, 0)]

class ColorSetter:
    def __init__(self, colors, cm=None):
        if len(colors.shape) == 2:
            self.each_f = True
            self.colors = colors
        else:
            self.each_f = False
            self.colors = np.array(list(map(lambda x: COLORS_ENUM[int(x)], colors)))
        self.cm = cm

    def __getitem__(self, i):
        if self.each_f:
            return self.cm.map(self.colors[i])
        else:
            return self.colors

class Updater:
    def __init__(self, frames, color_setter, sizes, points):
        self.frames = frames
        self.color_setter = color_setter
        self.points = points
        self.i = 1
        self.sizes = sizes

    def update(self, koeff):
        try:
            self.points.set_data(self.frames[self.i], 
                                 face_color     = self.color_setter[self.i],
                                 size           = self.sizes*koeff,
                                 edge_width     = None,
                                 edge_width_rel = 0.08)
            self.i += 1
        except:
            self.i = 0

def get_size(mol_type):
    if mol_type == 0:
        return 1.28
    elif mol_type == 1:
        return 1.91

if __name__ == '__main__':

    output_file = 'out.mp4'

    # Parsing arguments

    parser = argparse.ArgumentParser()

    parser.add_argument('frames', help='NumPy array of points coordinates.')
    parser.add_argument('colors', help='NumPy array of points velocities.')
    parser.add_argument('types', help='NumPy array of points velocities.')
    parser.add_argument('--fps', default=20, type=int)
    parser.add_argument('-r', '--realtime', default=True, type=int)
    parser.add_argument('-t', '--showtemp', default=True, type=int)
    parser.add_argument('--rotateangle', default=3., type=float)
    parser.add_argument('--cubesize', default='1000x1000x1000', type=str)
    parser.add_argument('--koeff', default=1.0, type=float)

    args = parser.parse_args()

    if (args.frames is None) or (args.colors is None) or (args.types is None):
        raise Exception('No files provided')

    # Load and transform data

    frames = np.load( args.frames )
    colors = np.load( args.colors )
    types = np.load( args.types )
    sizes = np.array([get_size(x) for x in types])

    colors /= colors.max()
    cm = vispy.color.Colormap(['lightblue', 'lightgreen', 'lightyellow', 'orange', 'red'], [0, 0.1, 0.3, 0.4, 1])

    if args.showtemp:
        color_setter = ColorSetter(colors, cm)
    else:
        color_setter = ColorSetter(types, cm)

    # Bureaucracy

    win = scene.SceneCanvas(keys='interactive',
                            size=(800, 800), show=True, bgcolor='white')

    view = win.central_widget.add_view()
    view.camera = 'arcball'

    coor_rang = np.abs(frames).max() * 1
    view.camera.set_range(x=[-coor_rang, coor_rang], y=[-coor_rang, coor_rang], z=[-coor_rang, coor_rang])

    # Real thing
    x_m, y_m, z_m = list(map(int, args.cubesize.split("x")))
    frames -= np.array([x_m/2, y_m/2, z_m/2])
    
    molecules = scene.visuals.Markers(parent=view.scene)
    molecules.set_data(frames[0], face_color=color_setter[0], size=sizes)

    borders = scene.visuals.Cube((x_m/2, y_m/2, z_m/2), color=[0.1, 0.1, 0.1, 0.1],
                               edge_color='black', parent=view.scene)
    ## Interactive animation

    if args.realtime:

        molecules.events.update.connect(lambda evt: win.update)

        upd = Updater(frames, color_setter, sizes, molecules)

        timer = vispy.app.Timer()
        timer.connect(lambda ev: upd.update(args.koeff * 1650 / view.camera.scale_factor))
        timer.start(interval=1/args.fps)

        if sys.flags.interactive != 1:
            vispy.app.run()


    ## Video creation
    else:
        import imageio

        writer = imageio.get_writer(output_file, fps=args.fps)

        axis = [0, 0, 1]

        for i in range(1, frames.shape[0]):
            im = win.render()
            writer.append_data(im)

            view.camera.transform.rotate(args.rotateangle, axis)

            molecules.set_data(frames[i], face_color=color_setter[i], size=sizes)

        writer.close()
