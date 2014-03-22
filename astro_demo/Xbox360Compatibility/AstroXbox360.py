def update():
   
   #RightTrigger  Fire
   keyboard.setKey(Key.F, xbox360[0].rightTrigger or (xbox360[0].rightStickX >  0.95))
   
   #LeftTrigger   Shift
   keyboard.setKey(Key.LeftShift, xbox360[0].leftTrigger)
   
   #RightBumper   Save
   keyboard.setKey(Key.Tab, xbox360[0].rightShoulder)
   
   #LeftBumper    LeftClick
   mouse.leftButton = xbox360[0].leftShoulder
   
   #LeftThumb     Wait
   keyboard.setKey(Key.NumberPad5, xbox360[0].leftThumb)
   
   #RightThumb    Look
   keyboard.setKey(Key.L, xbox360[0].rightThumb)

   #Esc        Menu
   keyboard.setKey(Key.Escape, xbox360[0].start)

   #A         Enter
   keyboard.setKey(Key.Return, xbox360[0].a)
   
   #Back      Stairs(With Shift)
   keyboard.setKey(Key.Period, xbox360[0].back)
   
   #B         Grab
   keyboard.setKey(Key.G, xbox360[0].b or (xbox360[0].rightStickY < -0.95))
   
   #X         Character Menu
   keyboard.setKey(Key.C, xbox360[0].x or (xbox360[0].down  and not xbox360[0].right and not xbox360[0].left))

   #LeftDPad      Drop
   keyboard.setKey(Key.D, xbox360[0].left and not xbox360[0].up and not xbox360[0].down)

   #Y      Inventory
   keyboard.setKey(Key.I, xbox360[0].y)
   
   #DPad(Up,Right,Down,Left)  InventorySelection(a-d)
   keyboard.setKey(Key.A, xbox360[0].up    and not xbox360[0].right and not xbox360[0].left)
   keyboard.setKey(Key.B, xbox360[0].right and not xbox360[0].up and not xbox360[0].down)
   #For C, see X
   #For D, see LeftDPad
   
   #RightStick(Up,Right,Down,Left) MoreInventorySelection(e-h)
   keyboard.setKey(Key.E, xbox360[0].rightStickY >  0.95)
   #For f, see RT
   #For g, see B
   keyboard.setKey(Key.H, xbox360[0].rightStickX < -0.95)

   #Movement   
   keyboard.setKey(Key.LeftArrow,  xbox360[0].leftStickX < -0.95) #left
   keyboard.setKey(Key.DownArrow,  xbox360[0].leftStickY < -0.95) #down
   keyboard.setKey(Key.RightArrow, xbox360[0].leftStickX >  0.95) #right
   keyboard.setKey(Key.UpArrow,    xbox360[0].leftStickY >  0.95) #up   
   keyboard.setKey(Key.NumberPad1, xbox360[0].leftStickY < -0.4 and xbox360[0].leftStickX < -0.4) #DownLeft
   keyboard.setKey(Key.NumberPad3, xbox360[0].leftStickY < -0.4 and xbox360[0].leftStickX >  0.4) #DownRight
   keyboard.setKey(Key.NumberPad7, xbox360[0].leftStickY >  0.4 and xbox360[0].leftStickX < -0.4) #UpLeft
   keyboard.setKey(Key.NumberPad9, xbox360[0].leftStickY >  0.4 and xbox360[0].leftStickX >  0.4) #UpRight

update()
    