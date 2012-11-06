<?php
/**
 * @file
 * All related user / admin functions (no hooks).
 */

/**
 * Is called by redirect when upload was done by MediaMosa.
 *
 * @param string $asset_id
 *   The ID of the asset.
 * @param string $mediafile_id
 *   The ID of the mediafile.
 */
function _media_mediamosa_uploaded_file($asset_id, $mediafile_id) {

  $fid = 0;

  try {
    // Need to get info of mediafile/asset.
    $mediafile_xml = MediaMosaCkConnectorWrapper::get_mediafile($mediafile_id);

    // Get filename.
    $filename = (string) $mediafile_xml->items->item->filename;

    // We need to build the scheme uri (mediamosa://asset/[id]/[id]/[filename]).
    $uri = strtr('mediamosa://asset/@asset_id/@mediafile_id/@filename', array(
      '@asset_id' => $asset_id,
      '@mediafile_id' => $mediafile_id,
      '@filename' => $filename,
    ));

    // Save it using uri.
    $file = file_uri_to_object($uri);
    file_save($file);

    // Get the file ID.
    $fid = $file->fid;
  }
  catch (Exception $e) {
    // If we fail, then don't create a file.
  }

  // Need to do this to close the popup.
  call_user_func_array('drupal_goto',
    array(
      'media/browser',
      array('query' => array('render' => 'media-popup', 'fid' => $fid)),
    )
  );
}
