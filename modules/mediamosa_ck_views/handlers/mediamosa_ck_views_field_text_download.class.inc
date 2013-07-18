<?php
/**
 * @file
 * Views field text download.
 */

class mediamosa_ck_views_field_text_download extends views_handler_field_xss {

  public function option_definition() {
    $options = parent::option_definition();

    $options['download_theme'] = array('default' => 'mediamosa_ck_views_theme_asset_download');
    $options['download_path'] = array('default' => 'asset/detail/[asset_id]');

    return $options;
  }

  public function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['download_theme'] = array(
      '#title' => t('Theme function to use'),
      '#type' => 'textfield',
      '#default_value' => $this->options['download_theme'],
    );
    $form['download_path'] = array(
      '#title' => t('Path of the asset'),
      '#type' => 'textfield',
      '#default_value' => $this->options['asset_path'],
      '#description' => t('Use [asset_id] to fill in the asset ID.'),
    );
  }

  /**
   * Get the value that's supposed to be rendered.
   *
   * @param object $values
   *   An object containing all retrieved values.
   * @param string $field
   *   Optional name of the field where the value is stored.
   */
  public function get_value($values, $field = NULL) {

    // Get data.
    $download = parent::get_value($values, $field);
    // Theme the list of files.
    return theme($this->options['download_theme'], array('download' => $download));
  }

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
}
