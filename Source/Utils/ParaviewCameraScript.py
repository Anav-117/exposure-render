from paraview.simple import GetActiveCamera

def start_cue(self): 
    cam = GetActiveCamera()
    print(str(cam.GetViewUp()[0]) + "," + str(cam.GetViewUp()[1]) + "," + str(cam.GetViewUp()[2]))
def tick(self):
    cam = GetActiveCamera()
    print(str(cam.GetPosition()[0]) + "," + str(cam.GetPosition()[1]) + "," + str(cam.GetPosition()[2]) + "," + str(cam.GetFocalPoint()[0]) + "," + str(cam.GetFocalPoint()[1]) + "," + str(cam.GetFocalPoint()[2]))

def end_cue(self): pass
