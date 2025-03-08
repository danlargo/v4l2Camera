# decode MP4/MOV/3Gp file structure

Quick and dirty attempt, lots of copied code and inefficiencies.

Look at the "walkMP4" folder, it uses a JSON file to define the MP4 structure and walks the file in a "generic" way.

To Do
-----

- add math to the output
- decode all the sections, right now there is a lot that is still TBD or unknown
- there are lots of custom sections in MOV and 3GP files that need some digging to decode.