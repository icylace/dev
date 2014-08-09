<?php

/**
 * =================
 * DEVELOPMENT TOOLS
 * =================
 *
 * These are some functions that aid in the development process.
 *
 * @copyright Copyright (c) 2014 Ron Martinez
 *            Except for `read_last_line()` which is the copyright of its owner.
 * @license   http://opensource.org/licenses/MIT   MIT License
 * @version   0.2.0
 * @link      https://github.com/icylace/dev
 */


define('DEV_LOG',                '/Users/icylace/others/MAMP PRO Logs/php_error.log');
define('DEV_AUTO_CLEAR_ENABLED', 'auto clear enabled');

// Used by `dump()`.
define('ALIGN_LEFT',                0);
define('ALIGN_RIGHT',               1);
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
    while ($char !== false && $char !== "\n" && $char !== "\r") {
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
  } else {
    error_log(var_export(DEV_AUTO_CLEAR_ENABLED, true));
  }
}


// -----------------------------------------------------------------------------


function step() {
  static $steps = 0;
  return '<!-- ====[ STEP ' . ++$steps . ' ]==== -->';
}


// -----------------------------------------------------------------------------


function backtrace() {
  c(trace(debug_backtrace()));
}


// -----------------------------------------------------------------------------


/**
 * Clears the error log.
 */
function clear_log($log_file = DEV_LOG) {
  file_put_contents($log_file, null);
}


// -----------------------------------------------------------------------------


/**
 * Based on:
 * http://stackoverflow.com/a/4254008/1935675
 */
function is_associative($array) {
  return is_array($array) && (bool)count(array_filter(array_keys($array), 'is_string'));
}


// -----------------------------------------------------------------------------
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
 *          The type of sorting to employ.  Defaults to null which
 *          prevents sorting.  Standard sort flags are defined here:
 *          ({@link http://us3.php.net/manual/en/function.sort.php})
 *        - 'theme' (string|array)
 * @param int $indent_level - The current indent level.  Usually this
 *                            should be left alone.
 *
 * @return string - The formatted data.
 *
 * @used-by trace()
 * @used-by c()
 * @used-by cc()
 *
 * @todo Handle closures.  `Requests::get()` might be a decent test.
 * @todo Handle objects better.  Laravel models seem good for testing.
 * @todo Refactor.
 *
 */
function dump($data, $options = [], $indent_level = 0) {
  $themes = [

    'json' => [

      // TODO
      // - add a way to properly escape Unicode characters

      'array_open'  => '[',
      'array_close' => ']',
      'array_empty' => '[]',

      'associative_array_open'  => '{',
      'associative_array_close' => '}',
      'associative_array_empty' => '{}',

      'object_open'  => '{',
      'object_close' => '}',
      'object_empty' => '{}',

      'string_key_open'       => '"',    'string_key_close' => '"',
      'string_open'           => '"',    'string_close'     => '"',
      'string_find'           => '"',    'string_replace'   => '\\"',
      'assignment'            => ':',    'assignment_align' => ALIGN_LEFT,
      'indent_unit'           => '  ',   'separator_unit'   => ' ',
      'key_separator'         => '',     'value_separator'  => ' ',
      'assignment_in_group'   => ': ',   'item_separator'   => ',',
      'last_item_separator'   => '',     'line_separator'   => PHP_EOL,
      'multiline_separator'   => PHP_EOL,
      'showing_array_indexes' => false,
      'null'                  => 'null',
    ],

    'minimal' => [
      'array_empty' => '[]',
      'array_open'  => '',
      'array_close' => '',

      'associative_array_empty' => '[]',
      'associative_array_open'  => '',
      'associative_array_close' => '',

      'object_empty' => '{}',
      'object_open'  => ':',
      'object_close' => '',

      'string_key_open'       => '',      'string_key_close' => '',
      'string_open'           => '\'',    'string_close'     => '\'',
      'string_find'           => '\'',    'string_replace'   => '\\\'',
      'assignment'            => '  ‹',   'assignment_align' => ALIGN_RIGHT,
      'indent_unit'           => '  ',    'separator_unit'   => ' ',
      'key_separator'         => ' ',     'value_separator'  => '  ',
      'assignment_in_group'   => '',      'item_separator'   => '',
      'last_item_separator'   => '',      'line_separator'   => PHP_EOL,
      'multiline_separator'   => PHP_EOL,
      'showing_array_indexes' => true,
      'null'                  => 'null',
    ],

    'pseudo-php' => [
      'array_empty' => 'array()',
      'array_open'  => 'array(',
      'array_close' => ')',

      'associative_array_empty' => 'array()',
      'associative_array_open'  => 'array(',
      'associative_array_close' => ')',

      'object_empty' => '{}',
      'object_open'  => '{',
      'object_close' => '}',

      'string_key_open'       => '\'',   'string_key_close' => '\'',
      'string_open'           => '\'',   'string_close'     => '\'',
      'string_find'           => '\'',   'string_replace'   => '\\\'',
      'assignment'            => '=>',   'assignment_align' => ALIGN_LEFT,
      'indent_unit'           => '',     'separator_unit'   => '',
      'key_separator'         => '',     'value_separator'  => '',
      'assignment_in_group'   => '=>',   'item_separator'   => ',',
      'last_item_separator'   => '',     'line_separator'   => '',
      'multiline_separator'   => PHP_EOL,
      'showing_array_indexes' => false,
      'null'                  => 'NULL',
    ],

    'space' => [
      'array_empty' => '[]',
      'array_open'  => '',
      'array_close' => '',

      'associative_array_empty' => '[]',
      'associative_array_open'  => '',
      'associative_array_close' => '',

      'object_empty' => '{}',
      'object_open'  => ':',
      'object_close' => '',

      'string_key_open'       => '',      'string_key_close' => '',
      'string_open'           => '\'',    'string_close'     => '\'',
      'string_find'           => '\'',    'string_replace'   => '\\\'',
      'assignment'            => '  ',    'assignment_align' => ALIGN_RIGHT,
      'indent_unit'           => '  ',    'separator_unit'   => ' ',
      'key_separator'         => ' ',     'value_separator'  => '  ',
      'assignment_in_group'   => '',      'item_separator'   => '',
      'last_item_separator'   => '',      'line_separator'   => PHP_EOL,
      'multiline_separator'   => PHP_EOL,
      'showing_array_indexes' => true,
      'null'                  => 'null',
    ],

  ];

  $multilines_style = isset($options['multilines_style']) ? $options['multilines_style'] : MULTILINES_REGULAR;
  $sort_style       = isset($options['sort_style'      ]) ? $options['sort_style'      ] : null;
  $theme            = isset($options['theme'           ]) ? $options['theme'           ] : null;

  if (!is_array($theme)) {
    $theme = is_string($theme) ? $themes[$theme] : $themes['space'];
  }

  //
  // Setup the render theme for use.  Default to a prettified JSON format.
  //

  // $group_type = '';
  // if (is_associative($data)) {
  //   $group_type = 'associative_array';
  // } elseif (is_array($data)) {
  //   $group_type = 'array';
  // } elseif (is_object($data)) {
  //   $group_type = 'object';
  // }

  $group_type = is_associative($data) ? 'associative_array'
              : is_array($data)       ? 'array'
              : is_object($data)      ? 'object'
              : '';

  // Create the local theme variables.
  $default        = $themes['json'];
  $theme_elements = array_keys($default);
  foreach ($theme_elements as $x) {
    $$x = isset($theme[$x]) ? $theme[$x] : $default[$x];
  }

  $output = [];

  // Pretty-print arrays and objects in a pseudo-JSON style.
  if (!empty($group_type)) {
    $group_empty = ${$group_type . '_empty'};
    $group_open  = ${$group_type . '_open' };
    $group_close = ${$group_type . '_close'};

    if (empty($data)) {
      $output[] = $group_empty;
    } else {
      if ($sort_style === SORT_ONE_LINER_VS_ARRAY) {
        //
        //  Sort the entries based on this ordering:
        //
        //  1.  Separate into string indexes group and numerical indexes group
        //      with the numerical indexes group in the ascended position.
        //
        //  2.  Within each group separate into non-empty/filled collection
        //      subgroup and "one-liner" subgroup.
        //
        //  3.  Within each subgroup alphabetize in ascending order.
        //

        $entries = [
          'numbered' => [
            'one_liners'         => [],
            'filled_collections' => [],
          ],
          'labelled' => [
            'one_liners'         => [],
            'filled_collections' => [],
          ],
        ];

        // Do steps 1 and 2.
        foreach ($data as $key => $value) {
          $a_filled_collection = (is_object($value) || is_array($value)) && !empty($value);
          $key_kind            = is_int($key)         ? 'numbered'           : 'labelled';
          $value_kind          = $a_filled_collection ? 'filled_collections' : 'one_liners';
          $entries[$key_kind][$value_kind][$key] = $value;
        }

        // Do step 3.
        ksort($entries['labelled']['one_liners'        ]);
        ksort($entries['labelled']['filled_collections']);
        ksort($entries['numbered']['one_liners'        ]);
        ksort($entries['numbered']['filled_collections']);

        // Apply the new ordering.
        $data = $entries['labelled']['one_liners'        ]
              + $entries['labelled']['filled_collections']
              + $entries['numbered']['one_liners'        ]
              + $entries['numbered']['filled_collections'];
      } elseif ($sort_style) {
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

      $output[]          = $group_open;
      $indent_spacing    = str_repeat($indent_unit, $indent_level);
      $group_indent = $indent_spacing . $indent_unit;

      end($data);
      $last_item_key = key($data);

      foreach ($data as $key => $value) {
        $a_filled_collection = (is_object($value) || is_array($value)) && !empty($value);
        $cur_item_separator  = $key === $last_item_key ? $last_item_separator : $item_separator;
        $separator           = $assignment_in_group;
        $key_label           = $string_key_open . $key . $string_key_close;
        $dumped_value        = dump($value, $options, $indent_level + 1);

        // Align values if they're not non-empty arrays or non-empty objects.
        if (!$a_filled_collection) {
          // Align the assignment symbol.
          $separation_gap = $longest_length - strlen($key);
          if ($assignment_align === ALIGN_LEFT) {
            $key_separation_gap   = 0;
            $value_separation_gap = $separation_gap;
          } else {
            $key_separation_gap   = $separation_gap;
            $value_separation_gap = 0;
          }
          $separator = str_repeat($key_separator, $key_separation_gap)
                     . $assignment
                     . str_repeat($value_separator, $value_separation_gap);

          // See if we're working with a multiline string.
          if (strpos($dumped_value, PHP_EOL) !== false) {
            if ($multilines_style === MULTILINES_BELOW) {
              $separator .= $multiline_separator;
            } else {
              if ($multilines_style === MULTILINES_INDENTED_BELOW) {
                $separator   .= $multiline_separator . $group_indent . $indent_unit;
                $value_indent = $indent_unit;
              } else {
                $separator   .= $value_separator;
                $value_indent = str_repeat($separator_unit, strlen($key_label . $assignment . $value_separator));
              }
              $lines          = explode(PHP_EOL, $dumped_value);
              $indented_lines = [];
              $value_indent   = $group_indent . $value_indent;
              for ($i = 0, $l = count($lines); $i < $l; ++$i) {
                if ($i > 0 && !empty($lines[$i]) && $lines[$i] !== "\r") {
                  $indented_lines[] = $value_indent . $lines[$i];
                } else {
                  $indented_lines[] = $lines[$i];
                }
              }
              $dumped_value = implode(PHP_EOL, $indented_lines);
            }
          } else {
            $separator .= $value_separator;
          }
        }

        if ($showing_array_indexes || !is_array($data) || !is_numeric($key)) {
          $output[] = $group_indent . $key_label . $separator . $dumped_value . $cur_item_separator;
        } else {
          $output[] = $group_indent . $dumped_value . $cur_item_separator;
        }
      }

      if (!empty($group_close)) {
        $output[] = $indent_spacing . $group_close;
      }
    }
  } elseif ($data === null) {
    $output[] = $null;
  } elseif (is_string($data)) {
    $data     = str_replace($string_find, $string_replace, $data);
    $output[] = $string_open . $data . $string_close;
  } else {
    $output[] = var_export($data, true);
  }

  return implode($line_separator, $output);
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
 * @param array $options    - Defined in `dump()`.
 *
 * @return string - The formatted data.
 *
 * @uses dump()
 *
 * Example usage:
 *   c(trace(debug_backtrace()));
 */
function trace($raw_trace, $args_shown = false, $options = []) {
  $indent_level = 0;
  $output       = [];

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
 * @param array  $options - Defined in `dump()`.
 *
 * @see cc()
 * @uses dump()
 *
 * Example usage:
 *   c($var0);
 *   c($var1, 'variable1');
 *
 * @todo See if we can get the passed variable's name using this:
 *       {@link http://stackoverflow.com/a/2414745/1935675}
 *       or this:
 *       {@link http://stackoverflow.com/a/2222404/1935675}
 */
function c($data, $label = '', $clear = false, $options = []) {
  if ($clear) {
    clear_log();
  }

  $output   = [];
  $output[] = str_repeat(':', 80);

  // Show the function that's doing the variable dump.
  $backtrace = debug_backtrace();
  if (isset($backtrace[1]['function'])) {
    $class    = isset($backtrace[1]['class']) ? $backtrace[1]['class'] . $backtrace[1]['type'] : '';
    $fn_name  = $class . $backtrace[1]['function'] . '()';
    $output[] = $fn_name . '  ' . str_repeat(':', 105 - strlen($fn_name));
  }

  $output[] = '';
  $output[] = (!empty($label) ? $label . '  «  ' : '') . dump($data, $options);
  $output[] = '';
  $output[] = '';

  // Remove whitespace at the end of lines.
  $lines  = explode(PHP_EOL, implode(PHP_EOL, $output));
  $output = [];
  foreach ($lines as $line) {
    $output[] = rtrim($line);
  }

  error_log(implode(PHP_EOL, $output));
}


// -----------------------------------------------------------------------------

/* *
// Coming soon...
c_start_set()
c_finish_set()
/* */

// -----------------------------------------------------------------------------


/**
 * A variant of `c()` which traces multiple variables.
 *
 * @param mixed $data    - An associative array of data to be traced.
 *                         The keys of which are the labels and the
 *                         values are the data.
 * @param bool  $clear   - Clears the error log before tracing.
 * @param array $options - Defined in `dump()`.
 *
 * @see c()
 * @uses dump()
 *
 * Example usage:
 *   cc([
 *     'variable2' => $var2,
 *     'variable3' => $var3,
 *   ]);
 *
 */
function cc($data, $clear = false, $options = []) {
  if (!is_array($data)) {
    error_log('cc() only works with an array of data to trace');
    return;
  }

  if ($clear) {
    clear_log();
  }

  $output   = [];
  $output[] = str_repeat(':', 80);

  // Show the function that's doing the trace.
  $backtrace = debug_backtrace();
  if (isset($backtrace[1]['function'])) {
    $fn_name  = $backtrace[1]['function'] . '()';
    $output[] = $fn_name . '  ' . str_repeat(':', 105 - strlen($fn_name));
  }

  foreach ($data as $key => $value) {
    $output[] = '';
    $output[] = ($key && is_string($key) ? $key . '  «  ' : '') . dump($value, $options);
  }

  $output[] = '';
  $output[] = '';

  // Remove whitespace at the end of lines.
  $lines  = explode(PHP_EOL, implode(PHP_EOL, $output));
  $output = [];
  foreach ($lines as $line) {
    $output[] = rtrim($line);
  }

  error_log(implode(PHP_EOL, $output));
}
