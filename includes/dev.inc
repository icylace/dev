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
 * @param bool  $multilines_below - Multilined data will be displayed below
 *                                  the variable that holds it.
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
  $themes = array(
    'standard' => array(
      'array_empty'          => '[]',
      'array_open'           => '[',
      'array_closing'        => ']',
      'object_empty'         => '{}',
      'object_open'          => '{',
      'object_closing'       => '}',
      'assignment'           => '‹',
      'indent_unit'          => '  ',
      'separator_unit'       => ' ',
      'separator_2_unit'     => '  ',
      'separator_2_newline'  => PHP_EOL,
      'collection_separator' => ' ',
    ),
    'minimal' => array(
      'array_empty'          => '[]',
      'array_open'           => '',
      'array_closing'        => '',
      'object_empty'         => '{}',
      'object_open'          => ':',
      'object_closing'       => '',
      'assignment'           => '‹',
      'indent_unit'          => '  ',
      'separator_unit'       => ' ',
      'separator_2_unit'     => '  ',
      'separator_2_newline'  => PHP_EOL,
      'collection_separator' => '',
    ),
  );

  $output               = array();
  $an_object            = is_object($data);
  // $theme                = $themes['standard'];
  $theme                = $themes['minimal'];
  $collection_type      = ($an_object ? 'object' : 'array');
  $collection_empty     = $theme[$collection_type . '_empty'];
  $collection_open      = $theme[$collection_type . '_open'];
  $collection_closing   = $theme[$collection_type . '_closing'];
  $assignment           = $theme['assignment'];
  $indent_unit          = $theme['indent_unit'];
  $separator_unit       = $theme['separator_unit'];
  $separator_2_unit     = $theme['separator_2_unit'];
  $collection_separator = $theme['collection_separator'];

  // Pretty-print arrays and objects in a pseudo-JSON style.
  if (is_array($data) || $an_object) {
    if (empty($data)) {
      $output[] = $collection_empty;
    }
    else {
      if ($sort_style === SORT_ONE_LINER_VS_ARRAY) {
        /**
         *  Sort the entries based on this ordering:
         *
         *  1.  Separate into string indexes group and numerical indexes group
         *      with the numerical indexes group in the ascended position.
         *
         *  2.  Within each group separate into non-empty/filled collection
         *      subgroup and "one-liner" subgroup.
         *
         *  3.  Within each subgroup alphabetize in ascending order.
         */

        $entries = array(
          'numbered' => array(
            'one_liners'         => array(),
            'filled_collections' => array(),
          ),
          'labelled' => array(
            'one_liners'         => array(),
            'filled_collections' => array(),
          ),
        );

        // Do steps 1 and 2.
        foreach ($data as $key => $value) {
          $a_filled_collection = (is_object($value) || is_array($value)) && !empty($value);
          $key_kind   = (is_int($key)         ? 'numbered'           : 'labelled');
          $value_kind = ($a_filled_collection ? 'filled_collections' : 'one_liners');
          $entries[$key_kind][$value_kind][$key] = $value;
        }

        // Do step 3.
        ksort($entries['labelled']['one_liners']);
        ksort($entries['labelled']['filled_collections']);
        ksort($entries['numbered']['one_liners']);
        ksort($entries['numbered']['filled_collections']);

        // Apply the new ordering.
        $data = $entries['labelled']['one_liners']
              + $entries['labelled']['filled_collections']
              + $entries['numbered']['one_liners']
              + $entries['numbered']['filled_collections'];
      }
      elseif ($sort_style) {
        ksort($data, $sort_style);
      }

      $longest_length = 0;

      // Find the longest appropriate key.
      foreach ($data as $key => $value) {
        $a_filled_collection = (is_object($value) || is_array($value)) && !empty($value);
        $key_length = strlen($key);
        // It counts if it's longer and doesn't have a non-empty container value.
        if ($key_length > $longest_length && !$a_filled_collection) {
          $longest_length = $key_length;
        }
      }

      $indent_spacing = str_repeat($indent_unit, $indent_level);

      $output[] = $collection_open;

      foreach ($data as $key => $value) {
        $a_filled_collection = (is_object($value) || is_array($value)) && !empty($value);
        $dumped_value = dump($value, $multilines_below, $sort_style, $indent_level + 1);
        $separator    = $collection_separator;
        $separator_2  = '';
        // Align values if they're not non-empty arrays or non-empty objects.
        if (!$a_filled_collection) {
          $separator = str_repeat($separator_unit, $longest_length - strlen($key) + 2) . $assignment;
          // See if we're working with a multiline string.
          if (strpos($dumped_value, PHP_EOL) !== FALSE) {
            if ($multilines_below) {
              $separator_2 = $separator_2_newline;
            }
            else {
              $separator_2    = $separator_2_unit;
              $lines          = explode(PHP_EOL, $dumped_value);
              $indented_lines = array();
              for ($i = 0, $l = count($lines); $i < $l; $i++) {
                $spacing = '';
                if ($i > 0 && !empty($lines[$i]) && $lines[$i] !== "\r") {
                  $spacing = $indent_spacing . str_repeat($separator_unit, $longest_length + 9);
                }
                $indented_lines[] = $spacing . $lines[$i];
              }
              $dumped_value = implode(PHP_EOL, $indented_lines);
            }
          }
          else {
            $separator_2 = $separator_2_unit;
          }
        }

        $output[] = $indent_spacing . $indent_unit . dump($key) . $separator . $separator_2 . $dumped_value;
      }

      if (!empty($collection_closing)) {
        $output[] = $indent_spacing . $collection_closing;
      }
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
  if ($clear) {
    clear_log();
  }

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

  error_log(implode(PHP_EOL, $output));
}


// -----------------------------------------------------------------------------


/**
 * A variant of `trace()` which traces multiple variables.
 *
 * @param mixed $data             - An associative array of data to be traced.
 *                                  The keys of which are the labels and the
 *                                  values are the data.
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
    error_log('traces() only works with an array of data to trace');
    return;
  }

  if ($clear) {
    clear_log();
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
    $output[] = '' . ($key && is_string($key) ? $key . '  «  ' : '') . dump($value, $multilines_below, $sort_style);
  }

  $output[] = '';
  $output[] = '';

  error_log(implode(PHP_EOL, $output));
}
