from pydub import AudioSegment
import numpy as np
import scipy.io.wavfile as wav
import os

def convert_mp3_to_wav(mp3_path, wav_path, target_sample_rate=16000, bit_depth=16):
    # Load MP3
    audio = AudioSegment.from_mp3(mp3_path)

    # Set frame rate (sampling rate) and sample width (bit depth)
    audio = audio.set_frame_rate(target_sample_rate)

    if bit_depth == 8:
        audio = audio.set_sample_width(1)
    elif bit_depth == 16:
        audio = audio.set_sample_width(2)
    elif bit_depth == 24:
        audio = audio.set_sample_width(3)
    elif bit_depth == 32:
        audio = audio.set_sample_width(4)
    else:
        raise ValueError("Unsupported bit depth. Use 8, 16, 24, or 32.")

    # Export to WAV
    audio.export(wav_path, format="wav")
    print(f"Converted MP3 to WAV: {wav_path}")

def read_wav_and_generate_header(wav_path, header_path, var_name="audio_data"):
    # Read WAV file
    sample_rate, data = wav.read(wav_path)
    print(f"Sample rate: {sample_rate}, Shape: {data.shape}, Dtype: {data.dtype}")

    # Flatten if stereo
    if data.ndim > 1:
        data = data[:, 0]  # Take only one channel

    # # Convert to list
    # data_list = data.tolist()

    unsigned_data = (data.astype(np.int32) + 32768).astype(np.uint16)

    unsigned_data = np.clip(unsigned_data, 0, 65535).astype(np.uint16)

    data_list = unsigned_data.tolist()

    # Write to .h file
    with open(header_path, 'w') as f:
        f.write(f"// Auto-generated audio data from {os.path.basename(wav_path)}\n")
        f.write(f"#ifndef {var_name.upper()}_H\n#define {var_name.upper()}_H\n\n")
        f.write(f"#define SAMPLE_RATE {sample_rate}\n")
        f.write(f"#define NUM_SAMPLES {len(data_list)}\n\n")
        f.write(f"unsigned short {var_name}[NUM_SAMPLES] = {{\n")

        for i, sample in enumerate(data_list):
            if i % 10 == 0:
                f.write("    ")
            f.write(f"{sample}, ")
            if (i + 1) % 10 == 0:
                f.write("\n")

        f.write("\n};\n\n#endif // {var_name.upper()}_H\n")
    print(f"Header file written to: {header_path}")

# === Example Usage ===
mp3_file = "Minecraft Splash - Sound Effect (HD).mp3"
wav_file = "splash.wav"
header_file = "splash_data.h"
sample_rate = 8000  # or any desired rate like 44100
bit_depth = 16       # 8, 16, 24, or 32

# convert_mp3_to_wav(mp3_file, wav_file, sample_rate, bit_depth)
read_wav_and_generate_header(wav_file, header_file)


# from pydub import AudioSegment

# audio = AudioSegment.from_file("input.mp3")
# audio.export("output.wav", format="wav")