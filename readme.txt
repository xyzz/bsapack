BSAPACK v 0.1a by ghostwheel
March 2, 2003

The Elder Scrolls 3 BSA file packer/unpacker.

PLEASE, READ THIS FILE ENTIRELY!


This utility performs 3 functions:

1) List content of the BSA file
2) Unpack BSA file into the current directory
3) Create new BSA file from files in the subdirectories in the current directory

This is the very first version of this utility and as such, it lacks many bells
and whistles. More precisely, it has VERY limited functionality and not user-friendly
interface. If you not familiar with command line utilities you better not use it.

This program has been tested extensively by me and should be stable at this point.
However, of course, I can not guarantee that there are no errors, so ALWAYS BACKUP your
Morrowind installation and ALWAYS run bsapack only in the designated directory, so it will
not be able to interfere with other files.

Source code (Visual C++) is also included, but it is not commented yet. Feel free to use it 
to write your own utility if you want. Once I will have time, I will prepare document that
describes BSA file format which should be helpful.

Finally, I am also not responsible for any actions this program will perform on your computer.
BSAPACK is distributed AS IS. 


Usage:

This is a windows console application, thus, all commands are specified through command line.



1) List files in the BSA archive:

bsapack list <archive_name>

This function will list file names in the archive together with some internal information.

Example:

bsapack list Tribunal.bsa



2) Unpack files from BSA archive into the current directory.

bsapack unpack <archive_name>

This function will unpack ALL files from the provided BSA archive into the current directory,
creating appropriate subdirectories for each file. At this time it is not possible to unpack
only certain files - all files will be unpacked always. So, PLEASE MAKE SURE that current
directory IS EMPTY before starting unpacking - never do that directly in Morrowind's
"Data Files" directory.

Example:

(create Temp directory on the D: drive and copy Tribunal.bsa and bsapack.exe into it)
(run command prompt)
D:
cd \Temp
bsapack unpack Tribunal.bsa



3) Pack files in the subdirectories in current directory into the new BSA archive.

bsapack pack <archive_name>

This function will create new BSA archive. ONLY files in all subdirectories (including nested
subdirs) in current directory will be added. Files, placed directly in the current directory,
WILL NOT be added to the archive. Also, at this time it is impossible to update existing BSA
archive - you can only create new one (but nothing prevents you from unpacking old archive,
modifying couple of files and creating new archive). Do not forget to add new BSA file into the
[Archives] section of the Morrowind.ini

Example:

(create Temp directory on the D: drive, copy bsapack.exe into it)
(create Icons, Meshes and Textures subdirectories in D:\Temp and put appropriate files into them)
(run command prompt)
D:
cd \Temp
bsapack pack NewFile.bsa
bsapack list NewFile.bsa >out.txt
(check content of the out.txt file to ensure that all files were indeed added to the NewFile.BSA)



IMPORTANT INFORMATION

1) BSA archive does not store date/time information about files. Thus, after unpacking all files
will have current date/time. This is a limitation of the BSA file format.

2) BSA archive ALWAYS stores all file names in lowercase. Thus, after unpacking all created
directories and files will be in the lowercase. This is also a limitation of the BSA file
format.

3) I tried to put various directories in the Morrowind "Data Files" directory into the BSA files.
So far, results are following:

- Fonts, Splash, Music and Sound directories SHOULD NOT be placed in the BSA archives. Morrowind will
  not find them there.

- Icons, Meshes and Textures directories can be freely placed into the single or multiple BSA archives.

- BookArt directory is empty in my installation, so I was not able to test it.

4) I never tested whether or not order in which BSA archives listed in the Morrowind.ini have any effects on the Morrowind. I also have not tested what will happen if same file was put into multiple
BSA archives (may that is when order will come into play).

5) Currently BSAPACK can add only up to 20000 files into single BSA archive. While this should be sufficient for most imaginable situations, this limit can be easily increased if necessary.
