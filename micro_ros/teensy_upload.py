Import("env")

if env.GetProjectOption("upload_protocol") == "teensy-cli":
    # Select Teensy 4.1 explicitly instead of the platform's generic MCU name.
    env.Replace(UPLOADERFLAGS=["--mcu=TEENSY41", "-w", "-v"])
    loader = '"$UPLOADER" $UPLOADERFLAGS $SOURCES'
    wait = '"$PYTHONEXE" -c "import time; time.sleep(2)"'
    # On this board the first transfer after reboot can fail, leaving HalfKay
    # active. Retry once without issuing another reboot request.
    env.Replace(UPLOADCMD=wait + " && (" + loader + " || (" + wait + " && " + loader + "))")
