Set WshShell=WScript.CreateObject("WSCript.shell")
RetCode=WshShell.Run("ImageMagick.exe",0,True)
RetCode=WshShell.Run("Video-recording.exe",0,False)