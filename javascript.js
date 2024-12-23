// ************************************************
window.addEventListener('DOMContentLoaded', init, false);

var versions, releases;
/**
 * Initializes the application by fetching firmware versions and releases,
 * then generates the select list, checks for supported versions, resets checkboxes,
 * and handles templates.
 *
 * @function init
 * @returns {void}
 * @throws {Error} If there is an error loading versions.
 */
function init() {
    handleTemplates();
    
    Promise.all([
        fetch('firmware/versions.json').then(response => response.json()),
        fetch('firmware/releases.json')
            .then(response => {
                if (response.status === 404) {
                    console.log('No releases found.');
                    return [];
                }
                return response.json();
            })
            .catch(() => [])
    ])
    .then(([versions, releases]) => {
        window.versions = versions;
        window.releases = releases;
        GenerateSelectList(versions, [] , false, true);
        checkSupported(); 
        resetCheckboxes(setManifest);
    })
    .catch(error => console.error('Error loading versions:', error));
}

/**
 * Updates the inner HTML of the element with the ID 'coms' to provide
 * instructions and links for installing drivers for common ESP board chips.
 * 
 * The instructions include:
 * - A prompt to install and select the correct COM port.
 * - A note about potentially missing drivers for the board.
 * - Links to drivers for CP2102 and CH34x chips.
 * - A reminder to ensure the USB cable supports data transfer.
 */
function showSerialHelp() {
    document.getElementById('coms').innerHTML = `Hit "Install" and select the correct COM port.<br><br>
    You might be missing the drivers for your board.<br>
    Here are drivers for chips commonly used in ESP boards:<br>
    <a href="https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers" target="_blank">CP2102 (square chip)</a><br>
    <a href="https://github.com/nodemcu/nodemcu-devkit/tree/master/Drivers" target="_blank">CH34x (rectangular chip)</a><br><br>
    Make sure your USB cable supports data transfer.<br><br>
    `;
}

/**
 * Extracts and returns the first path segment from the current URL's pathname.
 * This is typically used to get the repository name from a URL.
 *
 * @returns {string} The first path segment of the URL's pathname.
 */
function getRepositoryName() {
    const url = new URL(top.location.href);
    const pathSegments = url.pathname.split('/');
    const firstPathSegment = pathSegments.length > 1 ? pathSegments[1] : '';
    return firstPathSegment;
}

/**
 * Checks if the web install button has the 'install-unsupported' attribute.
 * If the attribute is present, it calls the `unsupported` function.
 */
function checkSupported() {
    if (document.getElementById('web-install-button').hasAttribute('install-unsupported')) unsupported();
}


/**
 * Displays a message indicating that the user's browser is not supported.
 * The message suggests trying on Desktop Chrome or Edge and provides a link to the GitHub releases page.
 */
function unsupported() {
    
    document.getElementById('flasher').innerHTML = `Sorry, your browser is not yet supported!<p/>
    Please try <b>Chrome</b>, <b>Edge</b> or <b>Opera</b>.
    <p/>
    Find binary files here:<br>
    <a href="https://github.com/tobiasfaust/` + getRepositoryName() + `/releases" target="_blank">
    <button class="btn" slot="activate">GitHub Releases</button>
    </a>`
}

/**
 * Returns a set of available variants for a given version number and ChipFamily.
 * 
 * @param {array} versions - content of versions.json
 * @param {array} releases - content of releases.json
 * @param {string} versionNumber - the build number of the version/release
 * @param {string} chipFamily - the current used chip family, empty string for all
 * 
 * @returns {Set} A set of available variants for the given version number and chip family.
 */
function getAvailableVariants(v, r, versionNumber, chipFamily = '') {
    let variants = new Set();
    v.forEach(version => {
    if ((!versionNumber || version.build == versionNumber) && (chipFamily == '' || version.chipFamilies.includes(chipFamily)) && version.variant) {
            variants.add(version.variant);
        }
    });

    r.forEach(release => {
        if ((!versionNumber ||  release.build == versionNumber) && (chipFamily == '' || release.chipFamilies.includes(chipFamily)) && release.variant) {
            variants.add(release.variant);
        }
    });

    return variants;

}

/**
 * Resets all radio buttons on the page accordently selected version.
 * 
 * This function selects all input elements of type "radio" and sets their
 * `checked` property to `false` and their `disabled` property to `false`.
 * It effectively unchecks and enables all radio buttons.
 */
function resetCheckboxes(onClickEvent) {
    // gehe durch die json date versions und releases. Suche alle möglichen Ausprägungen zum Key "variant"
    // und erstelle für jede Ausprägung ein radio button. Wenn der radio button ausgewählt wird, dann wird
    // die Funktion setManifest aufgerufen und der Wert des radio buttons wird als Parameter übergeben.
    
    // disable install button
    if (document.getElementById('web-install-div')) {
        document.getElementById('web-install-div').classList.add('disabled');
    }

    const radioButtonsContainer = document.getElementById('variants');
    radioButtonsContainer.innerHTML = ''; // Clear existing radio buttons

    const variants = getAvailableVariants(versions, [], document.getElementById('versions').value);

    // Create radio buttons for each variant if > 1
    if (variants.size > 1) {
        document.getElementById('versions').removeEventListener('change', onClickEvent); 

        variants.forEach(variant => {
            const radio = document.createElement('input');
            radio.type = 'radio';
            radio.name = 'variant';
            radio.value = variant;
            radio.id = variant;
            radio.classList.add('radio__input');
            radio.addEventListener('change', onClickEvent);
            radio.disabled = false;
            radio.checked = false;

            const label = document.createElement('label');
            label.classList.add('radio__label');
            label.setAttribute('for', variant);
            label.appendChild(document.createTextNode(variant));

            // Append the radio button and label to the container
            radioButtonsContainer.appendChild(radio);
            radioButtonsContainer.appendChild(label);
        });
    } else {
        document.getElementById('versions').addEventListener('change', onClickEvent);
        if (document.getElementById('web-install-div')) {
            document.getElementById('web-install-div').classList.remove('disabled');
        }
        onClickEvent();
        console.log('Only one variant found. Skipping radio buttons.');
    }
} 
    

/**
 * Generates a select list with grouped and sorted versions and releases.
 * 
 * @param {array} versions - content of versions.json
 * @param {array} releases - content of releases.json
 * @param {bool} UseReleases - bool, use release.json für prelive and master or use all versions from versions.json
 * @param {bool} PreSelectHighestBuild - bool, preselect the highest build number or not
 * 
 * The function performs the following steps:
 * 1. Groups the `versions` array by `stage`, including only those with the stage "development".
 * 2. Groups the `releases` array by `stage`.
 * 3. Sorts each group by the `build` number in descending order.
 * 4. Creates `optgroup` elements for each stage and `option` elements for each version/release.
 * 5. Appends the `optgroup` elements to the select element with the id 'versions'.
 */
function GenerateSelectList(v, r, useReleases=true, PreSelectHighestBuild=true) {

    const select = document.getElementById('versions');
    const stages = {};

    // Group by stage, only include development versions
    v.forEach(obj => {
        if ((useReleases && obj.stage == "development") || !useReleases) {
            if (!stages[obj.stage]) {
                stages[obj.stage] = {};
            }
            if (!stages[obj.stage][obj.build]) {
                stages[obj.stage][obj.build] = [];
            }

            stages[obj.stage][obj.build].push(obj);
        }
    });

    if (useReleases) {
        // Group by stage, for all releases
        r.forEach(obj => {
            if (!stages[obj.stage]) {
                    stages[obj.stage] = [];
                }
            if (!stages[obj.stage][obj.build]) {
                stages[obj.stage][obj.build] = [];
            }
            stages[obj.stage][obj.build].push(obj);
        });
    }

    // Sort each stage by build number in descending order
    for (const stage in stages) {
        for (const build in stages[stage]) {
            stages[stage][build].sort((a, b) => b.build - a.build);
        }
    }

    // store the highest build number
    let highestBuild = 0;
    if (PreSelectHighestBuild) {
        for (const stage in stages) {
            for (const build in stages[stage]) {
                if (parseInt(build, 10) > highestBuild) {
                    highestBuild = parseInt(build, 10);
                }
            }
        }
    }

    const o = document.createElement('option');
    o.value = "";
    o.disabled = true;
    o.selected = true;
    o.text = "Select Version";
    select.appendChild(o);

    // Create optgroups and options
    for (const stage in stages) {
        const optgroup = document.createElement('optgroup');
        optgroup.label = stage;
        
        for (const build in stages[stage]) {
            const uniqueBuild = stages[stage][build][0];
            const option = document.createElement('option');
            option.value = uniqueBuild.build;
            option.text = uniqueBuild.version + " (Build " + uniqueBuild.build + ")";
            if (PreSelectHighestBuild) 
                option.selected = parseInt(uniqueBuild.build, 10) == highestBuild;
            optgroup.appendChild(option);
        }
        select.appendChild(optgroup);
    }
}

/**
 * Sets the manifest attribute of the web install button based on the selected option 
 * in the versions dropdown and radiobutton.
 * 
 * This function retrieves the selected option from the dropdown menu with the ID 'versions', 
 * extracts its value (buildnumber), get value of optional radiobutton (variant)
 * and extract the right manifest from versions/releases and sets the correct 'manifest' attribute 
 * to the ID 'web-install-button'.
 */
function setManifest() {
    build = document.getElementById('versions').value;
    variant = document.querySelector('input[name="variant"]:checked')?.value || undefined

    let manifestPath;
    /*
    deactivated temporarely due to: Mixed Content: The page at 'https://tobiasfaust.github.io/test/' was loaded over HTTPS, but requested an insecure resource 'http://www.diefaeuste.de:30080/https://github.com/tobiasfaust/test/releases/download/v.2.5.1-PRE-12465535356/manifestAll-standard.json'. This request has been blocked; the content must be served over HTTPS.

    for (const release of releases) {
        if (release.build == build && (!variant || release.variant == variant)) {
            manifestPath = "http://www.diefaeuste.de:30080/" + release.manifest;
            break;
        }
    }
    */
    if (!manifestPath) {
        for (const version of versions) {
            if (version.build == build && (!variant || version.variant == variant)) {
                manifestPath = version.manifest;
                break;
            }
        }
    }

    if (manifestPath) {
        document.getElementById('web-install-button').setAttribute('manifest', manifestPath);
        document.getElementById('web-install-div').classList.remove('disabled');
        console.log('use Manifest: ', manifestPath);
    } else {
        console.error('Manifest not found for the selected build and variant.');
    }

    //console.log('Selected build:', build, 'Selected variant:', variant, 'Manifest path:', manifestPath);
}

/**
 * Replaces template placeholders in the innerHTML of all elements in the document.
 * 
 * This function searches through all elements in the document for occurrences of a specific template placeholder
 * (enclosed in double curly braces, e.g., {{REPOSITORY}}) within their innerHTML. When found, it replaces the 
 * placeholder with the corresponding value obtained from the `getRepositoryName` function.
 * 
 * The template placeholder and its replacement value are defined in the `elem` object.
 * 
 */
function handleTemplates() {
    const elem = {"template": "REPOSITORY", value: getRepositoryName() };
    document.querySelectorAll('*').forEach(node => {
        if (node.innerHTML.includes('{{' + elem.template + '}}')) {
            while (node.innerHTML.includes('{{' + elem.template + '}}')) {
                node.innerHTML = node.innerHTML.replace('{{' + elem.template + '}}', elem.value);
            }
        }
    });
}