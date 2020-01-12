The layout on disk is as follows

```
+------------------+
|Magic Value (0xFC)| <----+ Each block begins with a magic value
+------------------+
| Rec Size (byte)  |
+------------------+
| Rec Count (byte) |
+------------------+

      Record 1 (Rec size) <---+
                              + A block is a collection of records
      Record n (Rec size) <---+

+------------------+
|Magic Value (0xFC)|
+------------------+
| Rec Size (byte)  |
+------------------+
| Rec Count (byte) |
+------------------+
|Magic Value (0xFC)| <----+ The file ends with a magic value
+------------------+
             http://www.asciiflow.com/
```

A record is used to partially fill C structs. For complete structs the count should be 0 and the memory can be copied at once

