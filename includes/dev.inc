<?php
/**
 * =================
 * DEVELOPMENT TOOLS
 * =================
 *
 * These are some functions that aid in the development process.
 *
 * @copyright Copyright (c) 2013 Ron Martinez
 *            Except for `read_last_line()` which is the copyright of its owner.
 * @license   http://opensource.org/licenses/MIT   MIT License
 * @version   0.1.0
 * @link      https://github.com/icylace/dev
 */


define('DEV_LOG',                 '/Users/icylace/others/MAMP PRO Logs/php_error.log');
define('DEV_AUTO_CLEAR_ENABLED',  'auto clear enabled');
define('SORT_ONE_LINER_VS_ARRAY', 'one liner vs array');


// auto_clear_log();


// -----------------------------------------------------------------------------


function auto_clear_log() {
  $line = read_last_line(DEV_LOG);

  if (strpos($line, DEV_AUTO_CLEAR_ENABLED) > 0) {
    clear_log();
  }
  else {
    error_log(var_export(DEV_AUTO_CLEAR_ENABLED, TRUE));
  }
}


// -----------------------------------------------------------------------------


function backtrace() {
  trace(parse_backtrace(debug_backtrace()));
}


// -----------------------------------------------------------------------------


/**
 * Clears the error log.
 */
function clear_log() {
  file_put_contents(DEV_LOG, NULL);
}


// -----------------------------------------------------------------------------


/**
 * Renders data into a custom format.
 *
 * Loosely based on:
 * http://php.net/manual/en/function.var-export.php#54440
 *
 * @param mixed $data             - The data to render.
 * @param bool  $multilines_below - Make those parts of the data that will
 *                                  be formatted on multiple lines to be
 *                                  sorted at the end.
 * @param mixed $sort_style       - The type of sorting to employ.
 *                                  Set to NULL if sorting is not desired.
 *                                  Standard sort flags are defined here:
 *                                  ({@link http://us3.php.net/manual/en/function.sort.php})
 * @param int   $indent_level     - The current indent level.  Usually this
 *                                  should be left alone.
 *
 * @return string - The formatted data.
 *
 * @used-by parse_backtrace()
 * @used-by trace()
 * @used-by traces()
 *
 */
function dump($data, $multilines_below = FALSE, $sort_style = NULL, $indent_level = 0) {
  $dumping_object = FALSE;
  $output         = array();

  if (is_object($data)) {
    //
    // Convert the object into an array first for streamlined processing.
    // https://bugs.php.net/bug.php?id=48016&edit=1
    // http://stackoverflow.com/questions/2476876/convert-object-to-array/2476954#2476954
    //
    $data = json_decode(json_encode($data), TRUE);

    $dumping_object = TRUE;
  }

  // Pretty-print arrays and objects in a pseudo-JSON style.
  if (is_array($data)) {
    if (!empty($data)) {
      if ($sort_style === SORT_ONE_LINER_VS_ARRAY) {
        /**
         *  Sort the entries based on this ordering:
         *
         *  1.  Separate into string indexes group and numerical indexes group
         *      with the numerical indexes group in the ascended position.
         *
         *  2.  Within each group separate into non-empty container subgroup and
         *      "one-liner" subgroup.
         *
         *  3.  Within each subgroup alphabetize in ascending order.
         */

        $labelled_one_liners           = array();
        $labelled_non_empty_containers = array();
        $numbered_one_liners           = array();
        $numbered_non_empty_containers = array();

        foreach ($data as $key => $value) {
          // Do step 1.
          if (is_int($key)) {
            // Do step 2.
            if ((is_object($value) || is_array($value)) && !empty($value)) {
              $numbered_non_empty_containers[$key] = $value;
            }
            else {
              $labelled_one_liners[$key] = $value;
            }
          }
          else {
            // Do step 2.
            if ((is_object($value) || is_array($value)) && !empty($value)) {
              $labelled_non_empty_containers[$key] = $value;
            }
            else {
              $labelled_one_liners[$key] = $value;
            }
          }
        }

        // Do step 3.
        ksort($labelled_one_liners);
        ksort($labelled_non_empty_containers);
        ksort($numbered_one_liners);
        ksort($numbered_non_empty_containers);

        // Apply the new ordering.
        $data = $labelled_one_liners
              + $labelled_non_empty_containers
              + $numbered_one_liners
              + $numbered_non_empty_containers;
      }
      elseif ($sort_style) {
        ksort($data, $sort_style);
      }

      $longest_length = 0;

      // Find the longest appropriate key.
      foreach ($data as $key => $value) {
        $key_length = strlen($key);

        // It counts if it's longer and doesn't have a non-empty container value.
        if ($key_length > $longest_length && ((!is_object($value) && !is_array($value)) || empty($value))) {
          $longest_length = $key_length;
        }
      }

      $indent_spacing = str_repeat('  ', $indent_level);

      $output[] = ($dumping_object ? '{' : '[');

      foreach ($data as $key => $value) {
        $dumped_value = dump($value, $multilines_below, $sort_style, $indent_level + 1);
        $separator    = ' ';
        $separator_2  = '';

        // Align values if they're not non-empty arrays.
        if ((!is_object($value) && !is_array($value)) || empty($value)) {
          $separator = str_repeat(' ', $longest_length - strlen($key) + 2) . '‹';
          if (strpos($dumped_value, PHP_EOL) !== FALSE) {
            if ($multilines_below) {
              $separator_2 = PHP_EOL;
            }
            else {
              $separator_2    = '  ';
              $lines          = explode(PHP_EOL, $dumped_value);
              $indented_lines = array();
              for ($i = 0; $i < count($lines); $i++) {
                if ($i > 0 && !empty($lines[$i])) {
                  $indented_lines[] = $indent_spacing . str_repeat(' ', $longest_length + 9) . $lines[$i];
                }
                else {
                  $indented_lines[] = $lines[$i];
                }
              }
              $dumped_value = implode(PHP_EOL, $indented_lines);
            }
          }
          else {
            $separator_2 = '  ';
          }
        }

        $output[] = $indent_spacing . '  ' . dump($key) . $separator . $separator_2 . $dumped_value;
      }

      $output[] = $indent_spacing . ($dumping_object ? '}' : ']');
    }
    else {
      $output[] = ($dumping_object ? '{}' : '[]');
    }
  }
  else if ($data === NULL) {
    $output[] = 'null';
  }
  else {
    $output[] = var_export($data, TRUE);
  }

  return implode(PHP_EOL, $output);
}


// -----------------------------------------------------------------------------


/**
 * Parses and renders a backtrace.
 *
 * Based on:
 * {@link http://us2.php.net/manual/en/function.debug-backtrace.php#85915}
 *
 * @param array $raw_trace        - The backtrace to parse.
 * @param bool  $args_shown       - Renders arguments as well.
 * @param bool  $multilines_below - Defined in `dump()`.
 * @param mixed $sort_style       - Defined in `dump()`.
 *
 * @return string - The formatted data.
 *
 * @uses dump()
 *
 * Example usage:
 *   trace(parse_backtrace(debug_backtrace()));
 */
function parse_backtrace($raw_trace, $args_shown = FALSE, $multilines_below = FALSE, $sort_style = SORT_ONE_LINER_VS_ARRAY) {
  $indent_level = 0;
  $output       = array();

  foreach ($raw_trace as $entry) {
    $indent_spacing = str_repeat('  ', $indent_level);

    $output[] =   $indent_spacing . '       ' . $entry['function'] . '()';
    if (isset($entry['file'])) {
      $output[] = $indent_spacing . 'File:  ' . $entry['file'];
    }
    if (isset($entry['line'])) {
      $output[] = $indent_spacing . 'Line:  ' . $entry['line'];
    }
    if ($args_shown && isset($entry['args'])) {
      $output[] = $indent_spacing . 'Args:';
      $output[] = $indent_spacing . dump($entry['args'], $multilines_below, $sort_style);
    }

    $output[] = '';

    ++$indent_level;
  }

  return implode(PHP_EOL, $output);
}


// -----------------------------------------------------------------------------


/**
 * http://stackoverflow.com/questions/1510141/read-last-line-from-file/1510248#1510248
 */
function read_last_line($filename) {
  $line   = '';
  $f      = fopen($filename, 'r');
  $cursor = -1;

  fseek($f, $cursor, SEEK_END);
  $char = fgetc($f);

  // Trim trailing newline chars of the file.
  while ($char === "\n" || $char === "\r") {
    fseek($f, $cursor--, SEEK_END);
    $char = fgetc($f);
  }

  // Read until the start of file or first newline char.
  while ($char !== FALSE && $char !== "\n" && $char !== "\r") {
    // Prepend the new char.
    $line = $char . $line;
    fseek($f, $cursor--, SEEK_END);
    $char = fgetc($f);
  }

  return $line;
}


// -----------------------------------------------------------------------------


function step() {
  static $steps = 0;
  return '<!-- ====[ STEP ' . ++$steps . ' ]==== -->';
}


// -----------------------------------------------------------------------------


/**
 * Writes variable data into the error log in a formatted fashion.
 *
 * @param mixed  $data             - Defined in `dump()`.
 * @param string $label            - The label to associate with the data.
 * @param bool   $clear            - Clears the error log before tracing.
 * @param bool   $multilines_below - Defined in `dump()`.
 * @param mixed  $sort_style       - Defined in `dump()`.
 *
 * @see traces()
 * @uses dump()
 *
 * Example usage:
 *   trace($var0);
 *   trace($var1, 'variable1');
 *
 */
function trace($data, $label = '', $clear = FALSE, $multilines_below = FALSE, $sort_style = SORT_ONE_LINER_VS_ARRAY) {
  $output   = array();
  $output[] = str_repeat(':', 80);

  // Show the function that's doing the trace.
  $backtrace = debug_backtrace();
  if (isset($backtrace[1]['function'])) {
    $function_name = $backtrace[1]['function'] . '()';
    $output[]      = $function_name . '  ' . str_repeat(':', 105 - strlen($function_name));
  }

  $output[] = '';
  $output[] = (!empty($label) ? $label . '  «  ' : '') . dump($data, $multilines_below, $sort_style);
  $output[] = '';
  $output[] = '';

  if ($clear) {
    clear_log();
  }

  error_log(implode(PHP_EOL, $output));
}


// -----------------------------------------------------------------------------


/**
 * A variant of `trace()` which traces multiple variables.
 *
 * @param mixed $data             - Defined in `dump()`.
 * @param bool  $clear            - Clears the error log before tracing.
 * @param bool  $multilines_below - Defined in `dump()`.
 * @param mixed $sort_style       - Defined in `dump()`.
 *
 * @see trace()
 * @uses dump()
 *
 * Example usage:
 *   trace(array(
 *     'variable2' => $var2,
 *     'variable3' => $var3,
 *   ));
 *
 */
function traces($data, $clear = FALSE, $multilines_below = FALSE, $sort_style = SORT_ONE_LINER_VS_ARRAY) {
  if (!is_array($data)) {
    error_log('traces() only works with multiple data');
    return;
  }

  $output   = array();
  $output[] = str_repeat(':', 80);

  // Show the function that's doing the trace.
  $backtrace = debug_backtrace();
  if (isset($backtrace[1]['function'])) {
    $function_name = $backtrace[1]['function'] . '()';
    $output[]      = $function_name . '  ' . str_repeat(':', 105 - strlen($function_name));
  }

  foreach ($data as $key => $value) {
    $output[] = '';
    $output[] = ($key && is_string($key) ? $key . '  «  ' : '') . dump($value, $multilines_below, $sort_style);
  }

  $output[] = '';
  $output[] = '';

  if ($clear) {
    clear_log();
  }

  error_log(implode(PHP_EOL, $output));
}
