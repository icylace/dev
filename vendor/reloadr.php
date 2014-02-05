<?php

function glob_recursive($pattern, $flags = 0) {
  $files = glob($pattern, $flags);
  foreach (glob(dirname($pattern) . '/*', GLOB_ONLYDIR | GLOB_NOSORT) as $dir) {
    $files = array_merge($files, glob_recursive($dir . '/' . basename($pattern), $flags));
  }
  return $files;
}

$file_lists = array_map('glob_recursive', explode(',', $_SERVER['QUERY_STRING']));
$files      = array();

foreach ($file_lists as $file_list) {
  $files = array_merge($files, $file_list);
}
foreach ($files as &$file) {
  $file = filemtime($file);
}

header('Last-Modified: '. date('r', @max($files)));
