<?php
/**
 * @file
 * Views field text metadata.
 */

class mediamosa_ck_views_field_text_metadata extends views_handler_field {

  public function option_definition() {
    $options = parent::option_definition();

    $options['metadata_theme'] = array('default' => 'mediamosa_ck_view_theme_asset_metadata');
    $options['metadata_implode'] = array('default' => 0);
    $options['metadata_implode_string'] = array('default' => NULL);

    return $options;
  }

  public function render($values) {
    return $this->get_value($values);
  }

  public function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['metadata_theme'] = array(
      '#title' => t('Theme function to use'),
      '#type' => 'textfield',
      '#default_value' => $this->options['metadata_theme'],
    );

    $form['metadata_implode'] = array(
      '#type' => 'checkbox', 
      '#title' => t('Join multiple values'),
      '#description' => t('When the field has multiple values, join it into one string.'),
      '#default_value' => $this->options['metadata_implode'],
    );

    $form['metadata_implode_string'] = array(
      '#type' => 'textfield', 
      '#title' => t('Join string'), 
      '#description' => t('When the field contains multiple values and should be joined,
                           this string will be used. You can use <code>\n</code> for a newline.'),
      '#default_value' => $this->options['metadata_implode_string'],
      '#size' => 10,
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

    // Get the data.
    $technical_metadata = parent::get_value($values, $field);

    foreach ($technical_metadata as $key => $data) {
      if (is_array($data) && $this->shouldJoinValues()) {
        $technical_metadata[$key] = implode($this->getJoinString(), $data);
      }
    }

    return theme($this->options['metadata_theme'], array('metadata' => $technical_metadata));
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

  protected function shouldJoinValues() {
    return isset($this->options['metadata_implode']) ? $this->options['metadata_implode'] : FALSE;
  }

  protected function getJoinString() {
    $str = isset($this->options['metadata_implode_string']) ? $this->options['metadata_implode_string'] : '';
    return str_replace('\n', "\n", $str);
  }
}
