#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include <filesystem>


using json = nlohmann::json;
namespace fs = std::filesystem;

/**
 * Execute a command and return its output
 * Handles file paths with spaces by adding quotes
 */
std::string run_command(const std::vector<std::string>& cmd) {
    std::string full_cmd;
    for (const auto& part : cmd) {
        // Add quotes for file paths containing spaces
        if (part.find(' ') != std::string::npos) {
            full_cmd += "\"" + part + "\" ";
        } else {
            full_cmd += part + " ";
        }
    }

#ifdef _WIN32
    full_cmd += " 2>nul";
#else
    full_cmd += " 2>/dev/null";
#endif

    std::string result;
    char buffer[128];
    FILE* pipe = popen(full_cmd.c_str(), "r");
    if (!pipe) {
        std::cerr << "Error: Failed to execute command\n";
        return "";
    }

    while (fgets(buffer, sizeof(buffer), pipe)) {
        result += buffer;
    }

    int exit_code = pclose(pipe);
    if (exit_code != 0) {
        std::cerr << "Warning: Command exited with code " << exit_code << "\n";
    }

    return result;
}

/**
 * Count the number of existing subtitle streams in a video file
 */
int count_subtitles(const std::string& video_path) {
    std::vector<std::string> cmd = {
            "ffprobe", "-v", "error",
            "-select_streams", "s",
            "-show_entries", "stream=index",
            "-of", "json",
            video_path
    };

    std::string output = run_command(cmd);

    if (output.empty()) {
        std::cerr << "Warning: ffprobe output is empty. Check if the file exists or if ffprobe is in PATH.\n";
        return 0;
    }

    try {
        auto data = json::parse(output);
        return data.contains("streams") ? static_cast<int>(data["streams"].size()) : 0;
    } catch (const json::parse_error& e) {
        std::cerr << "Error parsing JSON output: " << e.what() << "\n";
        return 0;
    }
}

/**
 * Attach subtitle to video file with proper metadata preservation
 * Preserves all technical information including bitrates
 */
void attach_subtitle(const std::string& input_file, const std::string& subtitle_file,
                     const std::string& output_file, bool add_metadata,
                     bool clear_existing_subs, const std::string& sub_language, const std::string& value_metadata) {

    int count_subs = count_subtitles(input_file);

    std::vector<std::string> command = {
            "ffmpeg", "-y", "-nostdin",
            "-i", input_file
    };

    // Add subtitle file as input if provided
    if (!subtitle_file.empty()) {
        command.emplace_back("-i");
        command.push_back(subtitle_file);

        // Map streams: preserve original video and audio, handle subtitles
        if (clear_existing_subs) {
            // Map all streams except existing subtitles, then add new subtitle
            command.insert(command.end(), {"-map", "0", "-map", "-0:s", "-map", "1"});
            count_subs -= 1;
        } else {
            // Map all original streams and add new subtitle
            command.insert(command.end(), {"-map", "0", "-map", "1"});
        }

        // Set language metadata for the new subtitle stream
        command.push_back("-metadata:s:s:" + std::to_string(count_subs));
        command.push_back("language=" + sub_language);
    } else {
        // Only map original streams if no subtitle is being added
        command.insert(command.end(), {"-map", "0"});
    }

    // Preserve ALL metadata from original file
    // This includes technical metadata like bitrates, codec info, etc.
    command.insert(command.end(), {"-map_metadata", "0"});

    // Preserve metadata for each stream type individually
    // This ensures technical information like bitrates are maintained
//    command.insert(command.end(), {"-map_metadata:s:v", "0:s:v"});  // Video stream metadata
//    command.insert(command.end(), {"-map_metadata:s:a", "0:s:a"});  // Audio stream metadata
//    if (count_subs > 0) {
//        command.insert(command.end(), {"-map_metadata:s:s", "0:s:s"});  // Subtitle stream metadata
//    }

    // Add custom metadata if requested
    if (add_metadata) {
        command.insert(command.end(), {
                "-metadata", "title=" + value_metadata,
                "-metadata:s:v", "title=" + value_metadata,
                "-metadata:s:a", "title=" + value_metadata,
                "-metadata:s:s", "title=" + value_metadata
        });
    }


    std::string extension = output_file.substr(output_file.find_last_of('.') + 1);

    // Use stream copy to preserve quality and technical specifications
    // This ensures bitrates and other technical info remain unchanged
    command.insert(command.end(), {
            "-c:v", "copy",     // Copy video codec (preserves bitrate)
            "-c:a", "copy",     // Copy audio codec (preserves bitrate)
            "-c:s", extension == "mp4" ? "mov_text" : "copy",     // Copy subtitle codec
//            "-avoid_negative_ts", "make_zero",  // Handle timestamp issues
            output_file
    });

    // Debug: Show the command being executed
    std::cout << "Executing command: ";
    for (const auto& part : command) {
        std::cout << part << " ";
    }
    std::cout << "\n";

    run_command(command);
    std::cout << "Subtitle attached successfully. Output: " << output_file << std::endl;
}

/**
 * Display help information
 */
void show_help(const char* program_name) {
    std::cout << "Usage:\n";
    std::cout << program_name << " -i <input> [-s <subtitle>] [-o <output>] [-l <lang>] [-m] [--clear-subs]\n\n";
    std::cout << "Options:\n";
    std::cout << "  -i, --input     Input video file (required)\n";
    std::cout << "  -s, --subtitle  Subtitle file to attach\n";
    std::cout << "  -o, --output    Output file (default: same as input)\n";
    std::cout << "  -l, --lang      Subtitle language code (default: eng)\n";
    std::cout << "  -m, --metadata  Add custom metadata only\n";
    std::cout << "  --name-meta     name metadata for video and streams (default: ro-ox.com)\n";
    std::cout << "  --clear-subs    Remove existing subtitles before adding new ones\n";
    std::cout << "  -h, --help      Show this help message\n\n";
    std::cout << "Examples:\n";
    std::cout << "  " << program_name << " -i movie.mp4 -s subtitles.srt\n";
    std::cout << "  " << program_name << " -i movie.mp4 -m\n";
    std::cout << "  " << program_name << " -i movie.mp4 -s sub.srt -l spa --clear-subs\n";
}

int main(int argc, char* argv[]) {
    std::string input_file;
    std::string subtitle_file;
    std::string output_file;
    std::string lang = "eng";
    std::string metadata_value = "ro-ox.com";
    bool add_metadata = false;
    bool clear_subs = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "-i" || arg == "--input") && i + 1 < argc) {
            input_file = argv[++i];
        } else if ((arg == "-s" || arg == "--subtitle") && i + 1 < argc) {
            subtitle_file = argv[++i];
        } else if ((arg == "-o" || arg == "--output") && i + 1 < argc) {
            output_file = argv[++i];
        } else if ((arg == "-l" || arg == "--lang") && i + 1 < argc) {
            lang = argv[++i];
        } else if (arg == "-m" || arg == "--metadata") {
            add_metadata = true;
            // Check next argument
            if (i + 1 < argc) {
                std::string next_arg = argv[i + 1];
                if (!next_arg.empty() && next_arg[0] != '-') {
                    metadata_value = next_arg;
                    ++i;
                }
            }
        } else if (arg == "--clear-subs") {
            clear_subs = true;
        } else if (arg == "-h" || arg == "--help") {
            show_help(argv[0]);
            return 0;
        } else {
            std::cerr << "Unknown argument: " << arg << "\n";
            show_help(argv[0]);
            return 1;
        }
    }


    // Validate required arguments
    if (input_file.empty() || (!add_metadata && subtitle_file.empty())) {
        std::cerr << "Error: Input file is required. Use -s for subtitle or -m for metadata only.\n";
        show_help(argv[0]);
        return 1;
    }

    // Check if input file exists
    if (!fs::exists(input_file)) {
        std::cerr << "Error: Input file does not exist: " << input_file << "\n";
        return 1;
    }

    // Check if subtitle file exists (if provided)
    if (!subtitle_file.empty() && !fs::exists(subtitle_file)) {
        std::cerr << "Error: Subtitle file does not exist: " << subtitle_file << "\n";
        return 1;
    }

    // Set output file to input file if not specified
    bool use_temp = false;

    if (output_file.empty()) {
        fs::path input_path(input_file);
        fs::path temp_path = input_path.parent_path() / (input_path.stem().string() + ".temp" + input_path.extension().string());
        output_file = temp_path.string();
        use_temp = true;
    }


    // Warn if output file is the same as input file
    if (input_file == output_file) {
        std::cout << "Warning: Output file is the same as input file. Original will be overwritten.\n";
    }

    // Execute the subtitle attachment process
    try {
        attach_subtitle(input_file, subtitle_file, output_file, add_metadata, clear_subs, lang, metadata_value);
        // Check for overwrite
        if (use_temp) {
            try {
                fs::rename(output_file, input_file); // Overwrite original file
                std::cout << "Original file overwritten: " << input_file << "\n";
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Failed to overwrite original file: " << e.what() << "\n";
                return 1;
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}