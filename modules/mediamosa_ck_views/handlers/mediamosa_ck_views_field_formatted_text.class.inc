<?php
/**
 * @file
 * Views field text.
 */

class mediamosa_ck_views_field_formatted_text extends views_handler_field {
  // The following methods express strong SQLisms in the base field handler
  // class. Override and blank them out.
  public function pre_render(&$values) {
  }

  public function option_definition() {
    $options = parent::option_definition();
    $options['format'] = array('default' => NULL);
    return $options;
  }

  public function options_form(&$form, &$form_state) {
    $form['format'] = array(
      '#type' => 'select',
      '#title' => t('Format'),
      '#options' => $this->get_formats_list(),
      '#default_value' => isset($this->options['format']) ? $this->options['format'] : filter_default_format(),
    );

    parent::options_form($form, $form_state);
  }

  private function get_formats_list() {
    $formats = array();
    foreach (filter_formats() as $key => $format) {
      $formats[$key] = $format->name;
    }
    return $formats;
  }

  function render($values) {
    $value = $this->get_value($values);
    $format = isset($this->options['format']) ? $this->options['format'] : filter_default_format();
    return check_markup($value, $format);
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
