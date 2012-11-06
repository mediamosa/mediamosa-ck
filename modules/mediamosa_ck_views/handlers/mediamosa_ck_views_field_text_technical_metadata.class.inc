<?php
/**
 * @file
 * Views field text technical metadata.
 */

class mediamosa_ck_views_field_text_technical_metadata extends views_handler_field {

  public function option_definition() {
    $options = parent::option_definition();

    $options['technical_metadata_theme'] = array('default' => 'mediamosa_ck_view_theme_asset_technical_metadata');

    return $options;
  }

  public function render($values) {
    return $this->get_value($values);
  }

  public function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['technical_metadata_theme'] = array(
      '#title' => t('Theme function to use'),
      '#type' => 'textfield',
      '#default_value' => $this->options['technical_metadata_theme'],
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

    return theme($this->options['technical_metadata_theme'], array('technical_metadata' => $technical_metadata));
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
