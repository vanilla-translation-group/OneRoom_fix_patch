If MsgBox("Are you sure you want to uninstall patch?", vbYesNo) <> vbYes Then
	WScript.Quit
End If
Dim arr, fso
arr = Array("OneRoom_fix.exe", "ORPatch.dll", "override.xp3", "filelist.dat", "uninstall_fix.vbs")
Set fso = CreateObject("Scripting.FileSystemObject")
For Each file in arr
	fso.DeleteFile(file)
Next
MsgBox "Uninstalled."