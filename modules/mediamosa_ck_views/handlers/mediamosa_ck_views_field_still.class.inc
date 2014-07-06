<?php
/**
 * @file
 * Views field still.
 */

class mediamosa_ck_views_field_still extends views_handler_field {
  // The following methods express strong SQLisms in the base field handler
  // class. Override and blank them out.
  public function pre_render(&$values) {
  }

  public function add_additional_fields($fields = NULL) {
    foreach ($fields as $field) {
      $this->aliases[$field] = $this->query->add_field($this->table, $field);
    }
  }

  public function click_sort($order) {
    $this->query->add_orderby($this->table, $this->real_field, $order);
  }

  public function query() {
    $this->field_alias = $this->query->add_field($this->table, $this->real_field);

    // Add in additional fields.
    if (!empty($this->additional_fields)) {
      $this->add_additional_fields($this->additional_fields);
    }
  }

  /**
   * Setup default for options.
   */
  public function option_definition() {
    $options = parent::option_definition();

    $options['still_style'] = array('default' => TRUE);

    return $options;
  }

  /**
   * Still options.
   */
  public function options_form(&$form, &$form_state) {

    // File the options.
    $styles = array();
    foreach (image_styles() as $style_name => $image_style) {
      $styles[$style_name] = $style_name;
    }

    $form['still_style'] = array(
      '#title' => t('Select image style'),
      '#type' => 'select',
      '#options' => $styles,
      '#default_value' => $this->options['still_style'],
    );
    parent::options_form($form, $form_state);
  }

  /**
   * Render function.
   */
  public function render($values) {
    if (isset($values->vpx_still_url) && ($values->vpx_still_url != '')) {
      $path = pathinfo($values->vpx_still_url);
      $image = $path['dirname'] . '/' . $path['filename'] . ',' . $this->options['still_style'] . '.' . $path['extension'];

      // If the backend has a correctly configured image_style: like
      // mediamosa_sb_normal or the one chosen with options_form, use that one,
      // otherwise use image style on the frontend.
      $img_headers = get_headers($image);
      if (!substr($img_headers[0], -2) == 'OK') {
        return theme('image', array('path' => $path['dirname'] . '/' . $path['filename'] . ',' . $this->options['still_style'] . '.' . $path['extension']));
      }
      else {
        // No imagestyle on the backend, so create one ourself in the frontend.
        $public_image = 'public://mediamosa-still/' . $this->options['still_style'] . '/' . $values->asset_id . '.jpg';

        if (file_exists($public_image)) {

          // Check last modified date with the headers of the image.
          $img_headers = get_headers($values->vpx_still_url, TRUE);

          // Sanity check; this must be an image.
          if (substr($img_headers['Content-Type'], 0, 5) != 'image') {
            $public_image = 'public://mediamosa-still/' . $this->options['still_style'] . '/notfound.png';
            return theme('image', array('path' => $public_image));
          }

          // Determine if server has a newer image.
          $date_server_image = DateTime::createFromFormat('D, d M Y H:i:s O', $img_headers['Last-Modified']);
          $date_local_image = DateTime::createFromFormat('U', filemtime($public_image));
        }

        if (!file_exists($public_image) || ($date_server_image > $date_local_image)) {
          // No file created yet, so create an image style ourself.
          $dirname = drupal_dirname($public_image);
          file_prepare_directory($dirname, FILE_CREATE_DIRECTORY | FILE_MODIFY_PERMISSIONS);

          // Image style only works with local files, so copy first.
          $tmp_image = 'public://mediamosa-still/' . $values->asset_id . '.jpg';
          copy($values->vpx_still_url, $tmp_image);
          image_style_create_derivative(image_style_load($this->options['still_style']), $tmp_image, $public_image);
          unlink($tmp_image);
        }
        return theme('image', array('path' => $public_image));
      }
    }
    elseif (isset($values->vpx_still_url)) {
      // Still option is given, but no vpx_still_url is found, use some general
      // image.  General images are found in ck_views/images.
      $img_root = DRUPAL_ROOT . '/' . drupal_get_path('module', 'mediamosa_ck') . '/modules/mediamosa_ck_views/images/';

      if (isset($values->asset_type) && ($values->asset_type != '')) {
        if (in_array($values->asset_type, array('video', 'audio', 'image'))) {
          $image = $values->asset_type . '.png';
        }
        else {
          $image = 'notfound.png';
        }
      }
      elseif (isset($values->mediafile_mime_type)) {
        $mimepart = strstr($values->mediafile_mime_type, '/', TRUE);

        if (in_array($mimepart, array('video', 'audio', 'image', 'text'))) {
          $image = $mimepart . '.png';
        }
        else {
          $image = 'notfound.png';
        }
      }
      else {
        $image = 'notfound.png';
      }

      $public_image = 'public://mediamosa-still/' . $this->options['still_style'] . '/' . $image;
      if (!file_exists($public_image)) {

        // Make sure dir is right.
        $dirname = drupal_dirname($public_image);
        file_prepare_directory($dirname, FILE_CREATE_DIRECTORY | FILE_MODIFY_PERMISSIONS);

        // Create a image style version of the general image.
        image_style_create_derivative(image_style_load($this->options['still_style']), $img_root . $image, $public_image);

      }
      return theme('image', array('path' => $public_image));
    }
    else {
      // No still option was given in the rest call.
      // Use the highly inefficient MediaMosaStreamWrapper.
      $variables = array(
        'style_name' => $this->options['still_style'],
        'path' => MediaMosaStreamWrapper::mediamosa_getLocalThumbnailPath($values->asset_id),
      );
      return theme('image_style', $variables);
    }
  }
}
