# LinDroid_Share

LinDroid is program for sharing files directly between linux computers and android phones.

## Overview

- The program currently work only while both devices are connected to the same network.
- The project is built using C for the linux side and kotlin for the andriod, they communicate through LinDroid protocol.
- The mechanism of how LinDroid work is through discovery, when any of the two devices want to share a file it boadcast a UDP discovery packet and the other device reply, then they initiate a TCP connection to start the transfer process, the two programs are listening at port 8081 for UDP discovery packet and at port 8080/TCP to apply the file transfer logic.

## Installation and usage

First clone the repository:
```
git clone https://github.com/salshidadi/LinDroid_Share.git
```

### Linux 
Now change your directory to the linux folder:

```
cd LinDroid_Share/linux
```

Then compile the code:
```
gcc -Iinclude src/*.c -o airdrop
```

#### Configuration
You need to open the TCP socket on port 8080 and the UDP on 8081/8082 for the project to work, so in the terminal run the following command:
```
sudo ufw allow 8080/tcp
sudo ufw allow 8081:8082/udp
```

Now you car run the program to listen for incoming files:
```
./airdrop --daemon
```
This way your program is running as daemon process listening for file sharing request and saving files to your device.\
(**Note: you need to change line 90 in main.c to configure it to the path were you want your files to be saved**).


Now what if you want to share a file from your computer to the phone? \
you can either run the following command:
```
./airdrop --send <file path>
```
Or if you use linux mint you you can add LinDroid to be easily accessable whenever you right click any file you will find option for LinDroid and when you click it the transfer process will run in the background, you can achieve that by applying the next steps.

1. Open the terminal and run:
```
nano ~/.local/share/nemo/actions/airdrop.nemo_action
```
2. Now paste this inside the file (**replace your file path after Exec**):
```
[Nemo Action]
Active=true
Name=Send via Airdrop
Comment=Send this file to a peer device
Exec=<your file path from root to airdrop> --send "%F"
Icon-Name=network-transmit-receive
Selection=s
Extensions=any;
```

3. Now restart nemo (the file manager) so you can see the update:
```
nemo -q

```

Try to right click any file and you will see the option for "Send via Airdrop" when you click it the file will transfer automatically in the background.



### Android 

Now download the Android app to your phone:

1.Go to the Releases tab on the right side of this GitHub page.

2.Download the app-debug.apk.

Then install the app on your device:
Tap the downloaded APK file in your file manager to install it.
(Note: a pop up will probably appear saying the app is blocked since this is not downloaded from the Play Store just click "More details" then "Install anyway").

#### Configuration
You need to add LinDroid to your Quick Settings panel so you can easily turn the listening daemon on and off, so on your phone apply the following steps:

1.Swipe down from the top of your screen to fully open the Quick Settings panel.

2.Tap the edit icon (the pencil) to add a new button.

3.Find the LinDroid tile, drag it into your active buttons, and save.

Now you can run the program to listen for incoming files:
Simply pull down your Quick Settings and tap the LinDroid tile to turn it on.
This way your phone is running a background process listening for file sharing requests and saving files directly to your Downloads/LinDroid folder.

Now what if you want to share a file from your phone to the computer?

You click the share button on any file you will find an option for LinDroid and when you click it the transfer process will run in the background and that's it.
