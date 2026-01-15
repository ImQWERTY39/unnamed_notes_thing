# Unnamed Notes App

A 'high'-performance, infinite-canvas handwriting application written in C.
This project uses a gemini-copied spatial hash table and bitmapped tiles, designed for eventual porting to a tablet.


## Personal Note
Its been a little over 3 weeks at the time of writing this message that I've started this project.
I'm using SDL for the first time and rely entirely on AI for UI related stuff, but most of the other code are _definetly_ all my idea and the SDL part will be replaced by _my original_ ideas soon.
I expect to have a working prototype by December 2026. I haven't thought of a name for it yet, I'll be grateful for any non-AI contributions :)

_I'll write down the entire README once I'm done with the actual project. Gemini thinks really high of my project lmao_


## 🚀 Performance Features

* **Bitmapped Tiles:** Each 256x256 tile is stored as a 1-bit bitmap (256x4x8-byte chunks), making pixel operations and clearing incredibly fast.
* **Spatial Hashing:** Tiles are stored in a hash table with a capacity of 256, allowing for an "infinite" coordinate system with O(1) lookup.
* **Row-Major Rendering:** Optimized loop structures ensure CPU cache-friendly access to the SDL framebuffer by walking memory linearly.
* **Sparse File Storage:** Tiles are flushed to disk as separate binary files using row-masking, saving only the rows that contain actual data.


## ⌨️ Controls

* **P**: Pen Mode (Draw)
* **E**: Eraser Mode
* **A**: Pan Mode (Click and drag to move canvas)


## 💾 Storage Architecture (not even implemented yet LMAO)

The application treats each document as a directory. Each tile is serialized only when "dirty" into a compact binary format:
1. **Header (32 bytes):** A bitmask identifying which of the 256 rows contain ink.
2. **Data (Variable):** 32-byte segments for active rows only, significantly reducing the 8KB-per-tile memory footprint on disk.


## ⚖️ License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
