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
 * @version   0.2.0
 * @link      https://github.com/icylace/dev
 */


define('DEV_LOG',                 '/Users/icylace/others/MAMP PRO Logs/php_error.log');
define('DEV_AUTO_CLEAR_ENABLED',  'auto clear enabled');

// Used by `dump()`.
define('MULTILINES_REGULAR',        0);
define('MULTILINES_BELOW',          1);
define('MULTILINES_INDENTED_BELOW', 2);
define('SORT_ONE_LINER_VS_ARRAY',   'one liner vs array');


// auto_clear_log();


// -----------------------------------------------------------------------------


function auto_clear_log() {
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

  $line = read_last_line(DEV_LOG);

  if (strpos($line, DEV_AUTO_CLEAR_ENABLED) > 0) {
    clear_log();
  }
  else {
    error_log(var_export(DEV_AUTO_CLEAR_ENABLED, TRUE));
  }
}


// -----------------------------------------------------------------------------


function step() {
  static $steps = 0;
  return '<!-- ====[ STEP ' . ++$steps . ' ]==== -->';
}


// -----------------------------------------------------------------------------


function backtrace() {
  trace(parse_backtrace(debug_backtrace()));
}


// -----------------------------------------------------------------------------
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
 * @param mixed $data    - The data to render.
 * @param array $options - An associative array of render settings.
 *        - 'multilines_style' (int)
 *          Determines the style with which multiline strings are rendered.
 *          Defaults to MULTILINES_REGULAR.
 *          - MULTILINES_REGULAR: Multiple lines are indented to the right
 *            of the assignment symbol.
 *          - MULTILINES_BELOW: Multiple lines are rendered below the
 *            assignment symbol is also unindented.
 *          - MULTILINES_INDENTED_BELOW: Multiple lines are rendered below
 *            the assignment symbol is also indented.
 *        - 'sort_style' (int|string)
 *          The type of sorting to employ.  Defaults to NULL which
 *          prevents sorting.  Standard sort flags are defined here:
 *          ({@link http://us3.php.net/manual/en/function.sort.php})
 *        - 'theme' (string|array)
 * @param int $indent_level - The current indent level.  Usually this
 *                            should be left alone.
 *
 * @return string - The formatted data.
 *
 * @used-by parse_backtrace()
 * @used-by trace()
 * @used-by traces()
 *
 */
function dump($data, $options = array(), $indent_level = 0) {
  $themes = array(
    'minimal' => array(
      'array_empty'           => '[]',
      'array_open'            => '',
      'array_close'           => '',
      'object_empty'          => '{}',
      'object_open'           => ':',
      'object_close'          => '',
      'showing_array_indexes' => TRUE,
      'assignment'            => '  ‹',
      'indent_unit'           => '  ',
      'separator_unit'        => ' ',
      'multiline_separator'   => PHP_EOL,
      'key_separator_unit'    => ' ',
      'value_separator'       => '  ',
      'collection_separator'  => '',
      'string_key_open'       => '',
      'string_key_close'      => '',
      'string_open'           => '\'',
      'string_close'          => '\'',
      'null'                  => 'null',
    ),
    'pseudo-json' => array(
      'array_empty'           => '[]',
      'array_open'            => '[',
      'array_close'           => ']',
      'object_empty'          => '{}',
      'object_open'           => '{',
      'object_close'          => '}',
      'showing_array_indexes' => FALSE,
      'assignment'            => ':',
      'indent_unit'           => '  ',
      'separator_unit'        => ' ',
      'multiline_separator'   => PHP_EOL,
      'key_separator_unit'    => '',
      'value_separator'       => ' ',
      'collection_separator'  => ': ',
      'string_key_open'       => '"',
      'string_key_close'      => '"',
      'string_open'           => '"',
      'string_close'          => '"',
      'null'                  => 'null',
    ),
  );

  $multilines_style = (isset($options['multilines_style']) ? $options['multilines_style'] : MULTILINES_REGULAR);
  $sort_style       = (isset($options['sort_style'])       ? $options['sort_style']       : NULL);

  // Use a custom theme if defined or use a preset theme.
  $theme = $themes['minimal'];
  if (isset($options['theme'])) {
    $theme_option = $options['theme'];
    if (is_array($theme_option)) {
      $theme = $theme_option;
    }
    else if (is_string($theme_option)) {
      $theme = $themes[$theme_option];
    }
  }

  $output    = array();
  $an_object = is_object($data);

  // Setup the render theme for use.  Default to a pseudo-JSON format.
  $default               = $themes['pseudo-json'];
  $collection_type       = ($an_object ? 'object' : 'array');
  $collection_empty      = (isset($theme[$collection_type . '_empty']) ? $theme[$collection_type . '_empty'] : $default[$collection_type . '_empty']);
  $collection_open       = (isset($theme[$collection_type . '_open'])  ? $theme[$collection_type . '_open']  : $default[$collection_type . '_open']);
  $collection_close      = (isset($theme[$collection_type . '_close']) ? $theme[$collection_type . '_close'] : $default[$collection_type . '_close']);
  $showing_array_indexes = (isset($theme['showing_array_indexes'])     ? $theme['showing_array_indexes']     : $default['showing_array_indexes']);
  $assignment            = (isset($theme['assignment'])                ? $theme['assignment']                : $default['assignment']);
  $indent_unit           = (isset($theme['indent_unit'])               ? $theme['indent_unit']               : $default['indent_unit']);
  $separator_unit        = (isset($theme['separator_unit'])            ? $theme['separator_unit']            : $default['separator_unit']);
  $multiline_separator   = (isset($theme['multiline_separator'])       ? $theme['multiline_separator']       : $default['multiline_separator']);
  $key_separator_unit    = (isset($theme['key_separator_unit'])        ? $theme['key_separator_unit']        : $default['key_separator_unit']);
  $value_separator       = (isset($theme['value_separator'])           ? $theme['value_separator']           : $default['value_separator']);
  $collection_separator  = (isset($theme['collection_separator'])      ? $theme['collection_separator']      : $default['collection_separator']);
  $string_key_open       = (isset($theme['string_key_open'])           ? $theme['string_key_open']           : $default['string_key_open']);
  $string_key_close      = (isset($theme['string_key_close'])          ? $theme['string_key_close']          : $default['string_key_close']);
  $string_open           = (isset($theme['string_open'])               ? $theme['string_open']               : $default['string_open']);
  $string_close          = (isset($theme['string_close'])              ? $theme['string_close']              : $default['string_close']);
  $null                  = (isset($theme['null'])                      ? $theme['null']                      : $default['null']);

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

      // Find the longest appropriate key length.
      foreach ($data as $key => $value) {
        $a_filled_collection = (is_object($value) || is_array($value)) && !empty($value);
        $key_length = strlen($key);
        if ($key_length > $longest_length && !$a_filled_collection) {
          $longest_length = $key_length;
        }
      }

      $indent_spacing    = str_repeat($indent_unit, $indent_level);
      $collection_indent = $indent_spacing . $indent_unit;
      $output[]          = $collection_open;

      foreach ($data as $key => $value) {
        $a_filled_collection = (is_object($value) || is_array($value)) && !empty($value);
        $separator    = $collection_separator;
        $key_label    = $string_key_open . $key . $string_key_close;
        $dumped_value = dump($value, $options, $indent_level + 1);

        // Align values if they're not non-empty arrays or non-empty objects.
        if (!$a_filled_collection) {
          $separator = str_repeat($key_separator_unit, $longest_length) . $assignment;
          // See if we're working with a multiline string.
          if (strpos($dumped_value, PHP_EOL) !== FALSE) {
            if ($multilines_style === MULTILINES_BELOW) {
              $separator .= $multiline_separator;
            }
            else {
              if ($multilines_style === MULTILINES_INDENTED_BELOW) {
                $separator   .= $multiline_separator . $collection_indent . $indent_unit;
                $value_indent = $indent_unit;
              }
              else {
                $separator   .= $value_separator;
                $value_indent = str_repeat($separator_unit, strlen($key_label . $assignment . $value_separator));
              }
              $lines          = explode(PHP_EOL, $dumped_value);
              $indented_lines = array();
              $value_indent   = $collection_indent . $value_indent;
              for ($i = 0, $l = count($lines); $i < $l; $i++) {
                if ($i > 0 && !empty($lines[$i]) && $lines[$i] !== "\r") {
                  $indented_lines[] = $value_indent . $lines[$i];
                }
                else {
                  $indented_lines[] = $lines[$i];
                }
              }
              $dumped_value = implode(PHP_EOL, $indented_lines);
            }
          }
          else {
            $separator .= $value_separator;
          }
        }

        if ($showing_array_indexes || !is_array($data) || !is_numeric($key)) {
          $output[] = $collection_indent . $key_label . $separator . $dumped_value;
        }
        else {
          $output[] = $collection_indent . $dumped_value;
        }
      }

      if (!empty($collection_close)) {
        $output[] = $indent_spacing . $collection_close;
      }
    }
  }
  else if ($data === NULL) {
    $output[] = $null;
  }
  else if (is_string($data)) {
    $output[] = $string_open . $data . $string_close;
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
 * @param array $raw_trace  - The backtrace to parse.
 * @param bool  $args_shown - Renders arguments as well.
 * @param bool  $options    - Defined in `dump()`.
 *
 * @return string - The formatted data.
 *
 * @uses dump()
 *
 * Example usage:
 *   trace(parse_backtrace(debug_backtrace()));
 */
function parse_backtrace($raw_trace, $args_shown = FALSE, $options = array()) {
  $indent_level = 0;
  $output       = array();

  foreach ($raw_trace as $entry) {
    $indent_spacing = str_repeat('  ', $indent_level);
    $output[]       = $indent_spacing . '       ' . $entry['function'] . '()';
    if (isset($entry['file'])) {
      $output[] = $indent_spacing . 'File:  ' . $entry['file'];
    }
    if (isset($entry['line'])) {
      $output[] = $indent_spacing . 'Line:  ' . $entry['line'];
    }
    if ($args_shown && isset($entry['args'])) {
      $output[] = $indent_spacing . 'Args:';
      $output[] = $indent_spacing . dump($entry['args'], $options);
    }

    $output[] = '';

    ++$indent_level;
  }

  return implode(PHP_EOL, $output);
}


// -----------------------------------------------------------------------------


/**
 * Writes variable data into the error log in a formatted fashion.
 *
 * @param mixed  $data    - Defined in `dump()`.
 * @param string $label   - The label to associate with the data.
 * @param bool   $clear   - Clears the error log before tracing.
 * @param bool   $options - Defined in `dump()`.
 *
 * @see traces()
 * @uses dump()
 *
 * Example usage:
 *   trace($var0);
 *   trace($var1, 'variable1');
 *
 */
function trace($data, $label = '', $clear = FALSE, $options = array()) {
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
  $output[] = (!empty($label) ? $label . '  «  ' : '') . dump($data, $options);
  $output[] = '';
  $output[] = '';

  error_log(implode(PHP_EOL, $output));
}


// -----------------------------------------------------------------------------


/**
 * A variant of `trace()` which traces multiple variables.
 *
 * @param mixed $data    - An associative array of data to be traced.
 *                         The keys of which are the labels and the
 *                         values are the data.
 * @param bool  $clear   - Clears the error log before tracing.
 * @param bool  $options - Defined in `dump()`.
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
function traces($data, $clear = FALSE, $options = array()) {
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
    $output[] = '' . ($key && is_string($key) ? $key . '  «  ' : '') . dump($value, $options);
  }

  $output[] = '';
  $output[] = '';

  error_log(implode(PHP_EOL, $output));
}
