# TR-123e

**TR-123e is a monophonic embedded bass synthesiser built on the Bela real-time audio platform: a Zero Delay Feedback four-pole Moog ladder architecture, translated from a Max/MSP `gen~` patch into modular C++ running at ~1 ms latency on ARM.**

The design problem it addresses is the loss that usually occurs when software synthesis is moved into hardware: the expressiveness and responsiveness of the patch rarely survive the translation. TR-123e treats that translation itself as the research object — a systematic port from `gen~` to embedded DSP that preserves the sonic behaviour of the source while gaining the immediacy of a dedicated physical instrument with an eight-parameter hardware interface.

## Design position

TR-123e is the built groundwork of a wider research practice in open instrument design — instruments whose construction, signal path, and control surface are fully legible and modifiable by the performer, developed against the sealed commodity logic of the product synthesiser. The full design argument is set out in the accompanying dissertation (below) and the surrounding research is documented at [cybernics.co.uk](https://cybernics.co.uk). The instrument also integrates with TouchDesigner for real-time audiovisual performance.

## Key technical features

- **Zero Delay Feedback filter** — four-pole Moog ladder implementation (`zdf_moogladder_v2`), with comparative implementations and technical breakdowns in `DEV/`
- **Real-time performance on ARM** — stable, low-latency (~1 ms) operation on Bela
- **Modular C++ architecture** — discrete classes for envelope (`ADSR`), filter envelope (`MoogFilterEnvelope`), portamento (`PortamentoFilter`, `PortamentoPlayer`), key follow, velocity parsing, and MIDI handling, structured for modification and extension
- **Robust MIDI handling** — accurate event timing and portamento processing for continuous pitch transitions (tested with Arturia MiniLab 3)
- **Expressive hardware interface** — eight-parameter potentiometer control surface; wiring diagrams in `schematics/`
- **Audiovisual integration** — real-time parameter representation in TouchDesigner

## Repository contents

| Path | Contents |
|---|---|
| `.` | TR-123e synthesiser C++ codebase |
| `DEV/` | Comparative filter implementations, technical breakdowns, render setups |
| `schematics/` | Hardware interface design and wiring diagrams |

## Hardware requirements

- Bela platform (ARM-based embedded audio processor)
- MIDI controller (tested with Arturia MiniLab 3)
- Breadboard or custom control interface with potentiometers

## Installation and setup

1. Clone this repository:

   ```
   git clone https://github.com/gaialive/TR123e.git
   ```

2. Deploy the project onto your Bela board using the Bela IDE or terminal:

   ```
   scp -r TR123e root@bela.local:/root/Bela/projects/
   ```

3. Connect the hardware interface and MIDI controller according to the diagrams in `schematics/`.

## Usage

- Run TR-123e from the Bela IDE
- Adjust parameters using the hardware potentiometers for immediate sonic control
- Connect a MIDI controller for extended expressive control

## Documentation

Full interactive technical documentation (Doxygen): <https://www.gaialive.com/TR123e/html/>

The complete written study — design rationale, comparative filter analysis, and the open-instrument argument — is in the dissertation available on request.

## Contributing

Contributions and enhancements are welcome. Please submit issues or pull requests for review.

## License

TR-123e is distributed under the GAIALIVE License. See `LICENSE` for details.

## Author

**Timothy Paul Read** — [cybernics.co.uk](https://cybernics.co.uk) · <hello@cybernics.co.uk>

TR-123e originated as the final project of the BMus/BSc Electronic Music, Computing and Technology programme at Goldsmiths, University of London (First Class Honours, 2025).

© Timothy Paul Read 2025. All rights reserved.
