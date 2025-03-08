# Structured, data driver attempt to walk the MP4/MOV/3GP file structure.

- dictionary for the file structure is in [walkMP4_dictionary.json](./walkMP4_dictionary.json)


To Do
-----

- add math to the output
- decode all the sections, right now there is a lot that is still TBD or unknown
- there are lots of custom sections in MOV and 3GP files that need some digging to decode
- add conditional decoding (for MP4 time and MP4 duration etc), where data length is dependent on a version that is decoded in a previous step
    - this might start working when I get the moth working (i.e. assigning variables inside the JSON file).
- I need to make the JSON decoding "better", right now you can't use any keywords in the description fields or the parser breaks!!