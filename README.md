# Sorrygle-cpp

C++ Compiler for [Sorrygle](https://github.com/JJoriping/Sorrygle). (Works with limited grammars)

# Usage

```
sorrygle_compiler.exe filename.srg
```

## Notes

- It has only been tested on Windows and may not work in UNIX operating systems.

# Grammar

⭕: Supported
⚠️: Partially supported
❌: Not supported

| Notation           | Name                         |     |
| ------------------ | ---------------------------- | --- |
| `((bpm=ⓘ))`        | BPM configuration            | ⭕  |
| `((fermata=ⓝ))`    | Fermata configuration        | ❌  |
| `((time-sig=ⓘ/ⓘ))` | Time signature configuration | ❌  |
| `#ⓘ`               | Channel declaration          | ⭕  |
| `(o=ⓘ)`            | Default octave               | ⭕  |
| `(p=ⓘ)`            | Instrument                   | ⭕  |
| `(q=ⓓ)`            | Quantization                 | ⭕  |
| `(s=ⓘ)`            | Sustain pedal                | ❌  |
| `(t=ⓘ)`            | Transpose                    | ⭕  |
| `(v=ⓘ)`            | Volume                       | ⭕  |
| `cdefgab`          | Note                         | ⭕  |
| `CDFGA`            | Sharp note                   | ⭕  |
| `렢밒솚랖싶`       | Flat note                    | ❌  |
| `_`                | Rest                         | ⭕  |
| `+`                | Semitone up                  | ❌  |
| `-`                | Semitone down                | ❌  |
| `~`                | Tie                          | ⭕  |
| `[ceg]`            | Chord                        | ⭕  |
| `[\|ceg]`          | Arpeggio                     | ❌  |
| `^c`               | Octave up                    | ⭕  |
| `vc`               | Octave down                  | ⭕  |
| `(^ceg)`           | Ranged octave up             | ⭕  |
| `(vceg)`           | Ranged octave down           | ⭕  |
| `(3ceg)`           | Triplet                      | ⭕  |
| `(5cdefg)`         | Quintuplet                   | ❌  |
| `(7cdefgab)`       | Septuplet                    | ❌  |
| `(scdef)`          | Ranged sustain pedal         | ❌  |
| `[>ga]b`           | Appoggiatura                 | ⭕  |
| `[[c~~~\|efga]]`   | Parallelization              | ❌  |
| `<.c>`             | Staccato                     | ⭕  |
| `<~c>`             | Fermata                      | ❌  |
| `<!c>`             | Sforzando                    | ⭕  |
| `<tc>`             | Trill                        | ❌  |
| `<+cegⓘ>`          | Crescendo                    | ⭕  |
| `<-gecⓘ>`          | Decrescendo                  | ⚠️  |
| `<p(ⓝ)c~(ⓝ)~~(ⓝ)>` | Pitch bend                   | ❌  |
| `\|:`              | Open repeat                  | ⭕  |
| `:\|ⓘ`             | Close repeat                 | ⭕  |
| `/1`               | Prima volta                  | ❌  |
| `/2`               | Seconda volta                | ❌  |
| `{ⓘceg}`           | Group declaration            | ⭕  |
| `{=ⓘ}`             | Group reference              | ⚠️  |
| `=/`               | Head comment                 | ❌  |
| `/=`               | Tail comment                 | ⭕  |

# Dependencies

- [Midifile](https://github.com/craigsapp/midifile)

# License

MIT License

Copyright (c) 2022 이승훈
