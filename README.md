 MP3 Tag Reader and Editor

📌 Project Description
The MP3 Tag Reader and Editor is a command-line-based application developed using C programming.

This project is used to read and display MP3 file metadata (ID3 tags) and modify selected tag information 
such as Title, Artist, Album, Year, etc.

🚀 Features
- View MP3 file metadata.
- Edit MP3 ID3 tags.
- Supports command-line arguments.
- Handles file operations using C.
- Uses file handling functions to read and modify MP3 metadata.

🛠️ Technologies Used
- Programming Language: C
- Concepts: File Handling, Structures, Pointers, Command Line Arguments.
- Platform: Linux (Ubuntu)

📂 Project Structure
- `main.c` – Main program and command-line argument handling.
- `mp3view.c` – View MP3 metadata.
- `mp3view.h` – View function declarations.
- `mp3edit.c` – Edit MP3 metadata.
- `mp3edit.h` – Edit function declarations.

▶️ Compilation
gcc *.c 

▶️ Execution

To view MP3 details:
./a.out -v sample.mp3

To edit the title:
./a.out -e -t "New Title" sample.mp3

To edit the year:
./a.out -e -y "New Year" sample.mp3

To edit the album:
./a.out -e -A "New Album" sample.mp3

To edit the genre:
./a.out -e -m "New Genre" sample.mp3

To edit the artist:
./a.out -e -a "New Artist" sample.mp3

To edit the comments:
./a.out -e -c "New Comments" sample.mp3

🎯 Learning Outcomes
- Improved understanding of file handling in C.
- Practical implementation of pointers and structures.
- Understanding MP3 ID3 tag structure.
- Hands-on experience with command-line arguments.

👩‍💻 Author
Shravani Nitin Gurav
