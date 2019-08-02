@echo off
REM Modify the following variables to change the encoding options.
REM SET START_MINUTES=00
REM SET START_SECONDS=00
SET START_TIME=00
SET END_TIME=30
REM SET END_MINUTES=00
REM SET END_SECONDS=30
SET PATH=C:\Program Files\ffmpeg\bin;%path%
SET VidFadeOut=01
SET BumperTime=12.25


REM Use arugments passed to the script to override the above defaults.
FOR %%A IN (%*) DO (
	FOR /f "tokens=1,2 delims=:" %%G IN ("%%A") DO (
		set %%G=%%H
		ECHO %%G	%%H
	)
)


REM Set internal values based on parameters.
REM DOS only does integer arithmatic, so we are calling powershell here to support fractions of a second
REM The following line sets the START_TIME in seconds.
IF DEFINED START_MINUTES If DEFINED START_SECONDS (
	FOR /f  "delims=" %%x in  ('powershell %START_MINUTES%*60 + %START_SECONDS%') do SET START_TIME=%%x
)
IF DEFINED END_MINUTES If DEFINED END_SECONDS (
	FOR /f  "delims=" %%x in  ('powershell %END_MINUTES%*60 + %END_SECONDS%') do SET END_TIME=%%x
)
FOR /f  "delims=" %%x in  ('powershell %END_TIME% - %START_TIME%') do SET DURATION=%%x
FOR /f  "delims=" %%x in  ('powershell %END_TIME% - %VidFadeOut%') do SET FADE_OUT=%%x
FOR /f  "delims=" %%x in  ('powershell %START_TIME%+2') do SET BUMPER_START=%%x

REM Note that this audio office is not using the floating-point arithmatic
IF %START_TIME% LSS 0 SET /A AUDIO_OFFSET=-%START_TIME%
IF %START_TIME% GEQ 0 SET AUDIO_OFFSET=0
FOR /f  "delims=" %%x in  ('powershell %FADE_OUT% - %START_TIME%') do SET AUDIO_FADE_OUT=%%x

echo Output Video duration: %DURATION% seconds
echo[ 
echo[

REM Get list of MTS files and list them in SourceFiles.txt
REM @echo # Input files for the video encoder>SourceFiles.txt

setlocal enableextensions enabledelayedexpansion

IF DEFINED ProjectName (
	GOTO bumper
)

(
	SET /A FILE_COUNT=0
	FOR /F %%i in ('dir /b *.MTS ^| sort') do (
			SET ProjectName=%%~ni
			SET /A FILE_COUNT += 1
		)
)
REM >> SourceFiles.txt

REM If not MTS files found, check for the resulting mp4 file

IF %FILE_COUNT% EQU 0 (
	for %%i in (*.MP4) do (
			ECHO file '%%~ni.MP4'
			SET ProjectName=%%~ni
			SET /A FILE_COUNT += 1
		)
)

IF %FILE_COUNT% EQU 0 (
	for %%i in (*.M4V) do (
			ECHO file '%%~ni.M4V'
			SET ProjectName=%%~ni
			SET /A FILE_COUNT += 1
		)
)


REM Remove the last two characters from file name if there are multiple files. 
IF %FILE_COUNT% GTR 1 (
	SET ProjectName=!ProjectName:~0,-2!
)

:bumper

ECHO Project Name: !ProjectName!


REM Get the first PNG file and use it as the bumper image.
FOR %%F IN (*.png) DO (
	SET BumperImage=%%F
	GOTO encode
)

:encode


IF DEFINED EncodeSource ( IF !EncodeSource! NEQ 0 ( IF !EncodeSource! NEQ false (

	TITLE Transcoding !ProjectName!

	IF DEFINED BumperImage  (
		ECHO Using "!BumperImage!" for intro image.
		SET OVERLAY_FILTER=[1:v]fade=in:45:15,fade=out:314:30,setpts=PTS-STARTPTS+%START_TIME%/TB[intro];[v0][intro]overlay=eof_action=pass [v0];
		SET BumperImage=-i "!BumperImage!"
	) ELSE (
		ECHO No PNG image was found to use as the lower-third overlay.
		SET OVERLAY_FILTER=
	)
	REM Fade in and fade out. Add a short black clip at the end so that it fades all the way to black.
	REM SET COMMAND=ffmpeg -f concat -i SourceFiles.txt -loop 1 -framerate 30000/1001 !BumperImage! -filter_complex "color=c=black:s=1920x1080:d=2 [black_post]; [0:v]setpts=PTS-STARTPTS,fps=30000/1001,hqdn3d=4:4:2:3,fade=st=%START_TIME%:d=0.5,fade=t=out:st=%FADE_OUT%:d=%VidFadeOut% [v0]; !OVERLAY_FILTER! [v0] [black_post] concat [v]" -map "[v]:v" -map 0:a:0 -ss %START_TIME% -t %DURATION% -c:a copy -c:v libx264 -preset fast -crf 18 -loglevel info  "!ProjectName!.m4v"

	echo StartTime %START_TIME%
	echo Duration %DURATION%
	echo FADE_OUT %FADE_OUT%
	echo VidFadeOut %VidFadeOut%
	echo BumperTime %BumperTime%
	echo BumperImage %BumperImage%
	echo ProjectName %ProjectName%

	REM Remove the  -r 24000/1001  when there is a different input framerate.
	SET COMMAND=ffmpeg -f concat -safe 0 -t %END_TIME% -i SourceFiles.txt -itsoffset %BUMPER_START% -i ../../../CsehyVideoFooter.mov -i %BumperImage% -ss %START_TIME% -filter_complex "[0]scale=1920:1080,setpts=PTS-STARTPTS,fade=st=%START_TIME%:d=0.5,fade=t=out:st=%FADE_OUT%:d=%VidFadeOut% [video]; [video][1:v]overlay=eof_action=pass,setpts=PTS-STARTPTS[third];[third][2:v] overlay=x=(W-w)/2:y=if(lt(t\,%BUMPER_START%+5)\,H-h+max((%BUMPER_START%+0.8-t)*480\,0)\, H-h+max((-%BumperTime%-%BUMPER_START%-0.1+t)*360\,0)):enable='between(t,%BUMPER_START%+0.5,%BumperTime%+%BUMPER_START%+0.5)'" -af afade=t=out:st=%FADE_OUT%:d=%VidFadeOut% -c:a aac -b:a 320k -c:v libx264 -preset fast -crf 18 -tune film "%ProjectName%.m4v"
	
	echo[
	ECHO Running the following command:
	ECHO !COMMAND!
	
	!COMMAND!
	
	REM PAUSE
	
	echo[
	REM Could use -to for end time instead of -t for duration 
	echo[
	echo[
)))


IF DEFINED ExtractAudio IF %ExtractAudio% NEQ 0 IF !ExtractAudio! NEQ false (
	TITLE Extracting Audio from !ProjectName! for Editing 
	echo Extracting Audio for Editing
	ffmpeg  -i "!ProjectName!.m4v" -vn -af afade=t=out:st=%AUDIO_FADE_OUT%:d=1 "!ProjectName!.wav"
	echo[
)

REM If Extracting Audio and Recombining
IF DEFINED ExtractAudio IF %ExtractAudio% NEQ 0 IF %ExtractAudio% NEQ false IF DEFINED CombineAudio IF %CombineAudio% NEQ 0 IF %CombineAudio% NEQ false (
	TITLE Waiting for user confirmation 
	echo[
	echo Edit audio for "%ProjectName%", then continue to replace video audio with new audio
	PAUSE
	echo[
)

IF DEFINED CombineAudio (IF %CombineAudio% NEQ 0 IF %CombineAudio% NEQ false (
	TITLE Replacing audio with edited version on !ProjectName!
	echo Replacing audio with edited version
	ffmpeg  -i "%ProjectName%.m4v" -itsoffset %AUDIO_OFFSET% -i "%ProjectName%.wav" -map 0:0 -map 1:0 -vcodec copy -acodec aac -b:a 320k "!ProjectName!_normalizedAudio.m4v"
))

REM -loglevel warning

REM ffmpeg -i "%ProjectName%.m4v" -vn  -ss %START_TIME% -t %DURATION% -af apad "!ProjectName!.wav"


REM [0:v] fps=30000/1001 [main];[1:v] fade=t=out:st=5.5:d=0.5 [intro]; [main][intro] overlay=20:20

REM -c copy
REM ffmpeg -i SourceFiles.txt -i image.png -filter_complex "[0:v][1:v] overlay=25:25:enable='between(t,0,20)'" -pix_fmt yuv420p -c:a copy output.mp4



REM Extract Audio
IF DEFINED CreateAAC IF %CreateAAC% NEQ 0 IF %CreateAAC% NEQ false(
	ffmpeg -f concat -i SourceFiles.txt -y -vn -acodec copy "!ProjectName!.ac3" -ss %START_TIME% -t %DURATION% -acodec aac -q:a 0.26  -map_channel 0.1.0 -af "afade=t=in:ss=0:d=0.5" -vn "!ProjectName!.m4a"
)
REM ffmpeg -i !ProjectName!.ac3 -f wav -af "afade=t=in:ss=0:d=0.5" -map_channel 0.0.0 - | neroAacEnc -ignorelength -q 0.22 -if - -of "!ProjectName!_2.m4a"

:END