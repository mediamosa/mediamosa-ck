<?php
/**
 * @file
 * Media browser internet plugin, displays the upload form.
 */

class MediamosaMediaBrowserUpload extends MediaBrowserPlugin {

  /**
   * View function of Upload.
   */
  public function view() {
    $from_web_form = drupal_get_form('_media_mediamosa_upload_form', $this->params['types'], $this->params['multiselect']);
    return array(
      'form' => array($from_web_form),
      '#attached' => array(
        // 'js' => array($path . '/js/plugins/media.fromurl.js'),
      ),
    );
  }
}
