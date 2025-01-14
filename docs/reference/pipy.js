/**
 * Create a new Configuration object.
 *
 * @param {Object} [contextVariables] Context variables and their initial values.
 * @returns {Configuration} A new Configuration object.
 */
function pipy(contextVariables) {}

/**
 * Read a file's raw content from the codebase.
 *
 * @param {string} filename Path name of the file to read.
 * @returns {Data} Raw data of the file content.
 * @memberof pipy
 */
pipy.load = function(filename) {}

/**
 * Gracefully shuts down Pipy.
 *
 * @param {number} [exitCode] Exit code of the Pipy process. Defaults to zero.
 */
pipy.exit = function(exitCode) {}
