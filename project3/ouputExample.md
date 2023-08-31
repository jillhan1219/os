ouputExample
===
## Step 1
In step 1, the corresponding output of commands `I`, `R c s`, `W c s data` and `E` is shown in the following figure.
<img src="https://notes.sjtu.edu.cn/uploads/upload_47ba5af1ee2c9c3d46fcb79ae5430695.png" alt="Example Image" style="max-width: 500px;" />

## Step 2&3
As the ouput of step 2 and step 3 is almost the same, so I just show the output in step 3 of each commands.

- `f`, `mk f`, `w f l data`, `i f pos l data`, `d f pos l` and `cat f`
![](https://notes.sjtu.edu.cn/uploads/upload_c21fb5968ac352728cbe3fbc6314757d.png)
The pictures shows that when writing to the block or delete the content in the block, the disk receive the corresponding command and these operations are done in the disk.
- `mkdir d`, `cd`
![](https://notes.sjtu.edu.cn/uploads/upload_7e1c57137a38b6be007a1fcc9c45375d.png)
The picture shows 3 different cases of command `cd`.
- `rm f`, `ls`, `rmdir d`
    - `ls`
    <img src="https://notes.sjtu.edu.cn/uploads/upload_86d94508174638bf03b352d45e1002ee.png" alt="Example Image" style="max-width: 300px;" />
    - `rm f` and `rmdir`
    ![](https://notes.sjtu.edu.cn/uploads/upload_99803e57c314e5fb818e12e1a15e254f.png)
The picture shows that when removing a directory, the files and child directories are removed as well; It also shows that when there's content in the file, the corresponding content in disk will also be deleted.
- `e` and `f` when exits the file system and restarts it.
![](https://notes.sjtu.edu.cn/uploads/upload_56a8b3822b248f00fbf4108d162aead5.png)
It can be shown that when the file system restarts, the files and directories when the file system exits last time are persistent stored in the disk.
