function Invoke-CmdScript {
   param(
      [String] $script_path
   )
   Write-Output "running script $script_path"
   $cmdLine = """$script_path"" $args & set"
   & $Env:SystemRoot\system32\cmd.exe /c $cmdLine |
   select-string '^([^=]*)=(.*)$' | foreach-object {
      $varName = $_.Matches[0].Groups[1].Value
      $varValue = $_.Matches[0].Groups[2].Value
      set-item Env:$varName $varValue
   }
}

$vs_dir = "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise"
# $vs_dir = "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional"
$vcvars_dir = "{0}\VC\Auxiliary\Build\vcvars64.bat" -f $vs_dir

if((-Not (Test-Path env:cvars_invoked)) -And (-Not $env:cvars_invoked)){
   Invoke-CmdScript -script_path $vcvars_dir
   # prevent running that script more than once per session. It's slow and there's an issue with multiple invokations
   $env:cvars_invoked = $true
}

$cl_command = "CL /O2 /MDd /D _DEBUG /std:c++20 /EHsc /nologo /permissive- /W4 /wd4189 /utf-8 /Feout.exe source.cpp /link /MACHINE:X64"
$cl_cmd_zero = "CL /O2 /MDd /D _DEBUG /D ZERO /std:c++20 /EHsc /nologo /permissive- /W4 /wd4189 /utf-8 /Feout.exe source.cpp /link /MACHINE:X64"

# Measure zero includes
Invoke-Expression $cl_cmd_zero
$size=(Get-Item out.exe).length
Write-Output ("{0}, {1}" -f 0, $size) | Out-File -FilePath ("generated_data/compile_sizes_zero.txt") -Append -Encoding ASCII
Remove-Item -Path *.obj
Remove-Item -Path out.exe

For ($i=0; $i -lt 20; $i++){
   Start-Sleep -Seconds 0.2
   Measure-Command {
      Invoke-Expression $cl_cmd_zero
   } | Out-File -FilePath ("generated_data/compile_times_zero.txt") -Append -Encoding ASCII
   Remove-Item -Path *.obj
   Remove-Item -Path out.exe
}

$compressions = "none","zstd","lz4"

# Foreach($compression in $compressions){
#    Get-ChildItem "..\sample_datasets" -Filter *.png | Foreach-Object{
      
#       # Generate payload for compression and payload size
#       $config_path = "config_{0}.toml" -f $compression
#       $expr = "../x64/Release/binary_bakery.exe {0} {1}" -f $config_path, $_.FullName
#       Invoke-Expression $expr
#       Start-Sleep -Seconds 1.0

#       # One try first, check for errors and report size
#       Invoke-Expression $cl_command
#       $size=(Get-Item out.exe).length
#       Write-Output ("{0}, {1}" -f $_.Basename, $size) | Out-File -FilePath ("generated_data/compile_sizes_{0}.txt" -f $compression) -Append -Encoding ASCII
#       Remove-Item -Path *.obj
#       Remove-Item -Path out.exe

#       # Measure compile time multiple times
#       For ($i=0; $i -lt 20; $i++){
#          Start-Sleep -Seconds 0.2
#          Measure-Command {
#             Invoke-Expression $cl_command
#          } | Out-File -FilePath ("generated_data/compile_times_{0}_{1}.txt" -f $compression, $_.BaseName) -Append -Encoding ASCII
#          Remove-Item -Path *.obj
#          Remove-Item -Path out.exe
#       }

#    } # sample_datasets
# } # compressions