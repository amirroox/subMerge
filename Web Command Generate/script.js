let videoFile = null;
let subtitleFile = null;

// Drag and drop functionality
function setupDragDrop() {
    const dropZones = document.querySelectorAll('.drop-zone');

    dropZones.forEach(zone => {
        zone.addEventListener('dragover', (e) => {
            e.preventDefault();
            zone.classList.add('dragover');
        });

        zone.addEventListener('dragleave', () => {
            zone.classList.remove('dragover');
        });

        zone.addEventListener('drop', (e) => {
            e.preventDefault();
            zone.classList.remove('dragover');

            const files = e.dataTransfer.files;
            if (files.length > 0) {
                const file = files[0];
                if (zone.querySelector('#videoFile')) {
                    handleVideoFile(file);
                } else if (zone.querySelector('#subtitleFile')) {
                    handleSubtitleFile(file);
                }
            }
        });
    });
}

function handleVideoFile(file) {
    const videoExtensions = ['.mp4', '.avi', '.mkv', '.mov', '.wmv', '.flv', '.webm'];
    const extension = '.' + file.name.split('.').pop().toLowerCase();

    if (videoExtensions.includes(extension)) {
        videoFile = file;
        document.getElementById('videoFileName').textContent = file.name;
        document.getElementById('videoFileName').style.display = 'block';
    } else {
        alert('Please select a valid video file!');
    }
}

function handleSubtitleFile(file) {
    const subtitleExtensions = ['.srt', '.ass', '.ssa', '.vtt'];
    const extension = '.' + file.name.split('.').pop().toLowerCase();

    if (subtitleExtensions.includes(extension)) {
        subtitleFile = file;
        document.getElementById('subtitleFileName').textContent = file.name;
        document.getElementById('subtitleFileName').style.display = 'block';
    } else {
        alert('Please select a valid subtitle file!');
    }
}

// File input handlers
document.getElementById('videoFile').addEventListener('change', (e) => {
    if (e.target.files.length > 0) {
        handleVideoFile(e.target.files[0]);
    }
});

document.getElementById('subtitleFile').addEventListener('change', (e) => {
    if (e.target.files.length > 0) {
        handleSubtitleFile(e.target.files[0]);
    }
});

function generateCommand() {
    const statusSection = document.getElementById('statusSection');
    const statusMessage = document.getElementById('statusMessage');
    const commandOutput = document.getElementById('commandOutput');
    const commandText = document.getElementById('commandText');

    // Validation
    if (!videoFile) {
        showStatus('Please select a video file!', 'error');
        return;
    }

    const metadataOnly = document.getElementById('metadataOnly').checked;
    if (!metadataOnly && !subtitleFile) {
        showStatus('Please select a subtitle file or enable metadata only mode!', 'error');
        return;
    }

    // Build command
    let command = "";
    let command_cpp = './subMerge.exe';
    let command_python = 'python main.py';

    // Add video file
    command += ` -i "${videoFile.name}"`;

    // Add subtitle file if not metadata only
    if (!metadataOnly && subtitleFile) {
        command += ` -s "${subtitleFile.name}"`;
    }

    // Add output file if specified
    const outputPath = document.getElementById('outputPath').value.trim();
    if (outputPath) {
        command += ` -o "${outputPath}"`;
    }

    // Add language
    const language = document.getElementById('language').value;
    if (language !== 'eng') {
        command += ` -l ${language}`;
    }

    // Add metadata
    const addMetadata = document.getElementById('addMetadata').checked;
    if (addMetadata) {
        const metadataValue = document.getElementById('metadataValue').value.trim();
        if (metadataValue) {
            command += ` -m "${metadataValue}"`;
        } else {
            command += ` -m`;
        }
    }

    // Add clear subs option
    const clearSubs = document.getElementById('clearSubs').checked;
    if (clearSubs) {
        command += ` --clear-subs`;
    }

    // Add metadata only option
    if (metadataOnly) {
        command += ` -m`;
    }

    // Display command
    commandText.innerHTML = `<strong>Generated Command CPP:</strong><br><br>${command_cpp}${command}<br><br><br>`;
    commandText.innerHTML += `<strong>Generated Command Python:</strong><br><br>${command_python}${command}`;
    commandOutput.style.display = 'block';

    showStatus('Command generated successfully! Copy and run the command above.', 'success');
}

function showStatus(message, type) {
    const statusSection = document.getElementById('statusSection');
    const statusMessage = document.getElementById('statusMessage');

    statusSection.className = `status-section status-${type}`;
    statusMessage.textContent = message;
    statusSection.style.display = 'block';

    if (type === 'processing') {
        document.getElementById('progressBar').style.display = 'block';
    } else {
        document.getElementById('progressBar').style.display = 'none';
    }
}

// Initialize
setupDragDrop();
