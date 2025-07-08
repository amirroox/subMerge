import argparse
import subprocess
import os
import sys
import json

RESET = "\033[0m"
RED = "\033[31m"
GREEN = "\033[32m"
YELLOW = "\033[33m"
BLUE = "\033[34m"


def count_subtitles(video_path) -> int:
    command = [
        "ffprobe", "-v", "error",
        "-select_streams", "s",
        "-show_entries", "stream=index",
        "-of", "json",
        video_path
    ]
    result = subprocess.run(command, capture_output=True, text=True)
    data = json.loads(result.stdout)
    return len(data.get("streams", []))



def attach_subtitle(input_file, subtitle_file, output_file, add_metadata, clear_existing_subs, sub_language, name_mata):
    # Subtitle Counter
    count_subs = count_subtitles(input_file)

    # Main Command
    command = [
        "ffmpeg", "-y", "-nostdin",
        "-i", input_file
    ]

    # Map streams
    if subtitle_file:
        command += ["-i", subtitle_file]
        if clear_existing_subs:
            command += ["-map", "0", "-map", "-0:s", "-map", "1"]
            count_subs -= 1
        else:
            command += ["-map", "0", "-map", "1"]
        command += [f"-metadata:s:s:{count_subs}", f"language={sub_language}"]
    else:
        command += ["-map", "0"]

    # Keep original metadata
    command += ["-map_metadata", "0"]

    # Change MetaData
    if add_metadata:
        command += [
            "-metadata", f"title={name_mata}",
            "-metadata:s:v", f"title={name_mata}",
            "-metadata:s:a", f"title={name_mata}",
            "-metadata:s:s", f"title={name_mata}",
        ]

    # Use specific codec mapping
    command += [
        "-c:v", "copy",
        "-c:a", "copy",
        "-c:s", "mov_text" if output_file.endswith(".mp4") else "copy",
        # "-c:s", "mov_text",  # Optional: if using MP4
        output_file
    ]

    try:
        print(f"{BLUE}Executing command: {" ".join(command)}\n{RESET}")
        # Run Main
        subprocess.run(command, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        print(f"{GREEN}Subtitle attached successfully. Output: {output_file}{RESET}")
    except subprocess.CalledProcessError:
        print(f"{RED}Error occurred while running ffmpeg.{RESET}")
        sys.exit(1)


def main():
    # Arguments
    parser = argparse.ArgumentParser(description="Attach subtitle to video using FFmpeg")
    parser.add_argument("-i", "--input", required=True, help="Input video file")
    parser.add_argument("-s", "--subtitle", help="Subtitle file (.srt)")
    parser.add_argument("-l", "--lang", default="English", help="Language for subtitle (e.g., eng, fa)")
    parser.add_argument("-o", "--output", help="Output video file (default: overwrite input)")
    parser.add_argument("-m", "--metadata",const="ro-ox.com", nargs="?",
                        help="Add metadata to video and streams (default: ro-ox.com if no value is provided)")
    parser.add_argument("--clear-subs", action="store_true",
                        help="Remove existing subtitle tracks before adding the new one")

    args = parser.parse_args()

    input_file = args.input
    subtitle_file = args.subtitle

    if args.output:
        output_file = args.output
    else:
        # Save to temporary file, then replace original
        base, ext = os.path.splitext(input_file)
        output_file = base + ".temp" + ext

    # output_file = args.output or input_file
    meta_value = args.metadata  # None, "ro-ox.com" or custom value
    add_metadata = meta_value is not None
    clear_existing_subs = args.clear_subs
    subtitle_lang = args.lang

    if not os.path.exists(input_file):
        print("Input file does not exist.")
        sys.exit(1)

    if not add_metadata:
        if not subtitle_file or not os.path.exists(subtitle_file):
            print(f"{RED}Subtitle file is required and must exist unless using -m only.{RESET}")
            sys.exit(1)

    # Main Function
    attach_subtitle(input_file, subtitle_file, output_file, add_metadata, clear_existing_subs, subtitle_lang, meta_value)

    # Replace original file if no output was specified
    if not args.output:
        os.replace(output_file, input_file)
        print(f"{YELLOW}Original file replaced with updated file: {input_file}{RESET}")


if __name__ == "__main__":
    main()
