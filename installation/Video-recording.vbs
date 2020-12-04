Set WshShell=WScript.CreateObject("WSCript.shell")
WshShell.CurrentDirectory = "C:\Video-recording"
RetCode=WshShell.Run("Video-recording.exe",0,False)