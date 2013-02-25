<?php
/**
 * @file
 * All non-hooks are put here.
 */

/**
 * Default upload URL that prevents upload to Drupal.
 *
 * This function is attached to mediamosa/ck/upload/die. It can only be called
 * when upload has not been directed to MediaMosa, but to this front end. Will
 * do die() to stop any process of the uploaded file.
 */
function _mediamosa_ck_upload_die() {
  die('{"jsonrpc" : "2.0", "error" : {"code": 104, "message": "Access to local upload denied."}, "id" : "id"}');
}
