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

  public function render($values) {
    $variables = array(
      'style_name' => $this->options['still_style'],
      // @todo: should be an real path instead of public://.
      'path' => MediaMosaStreamWrapper::mediamosa_getLocalThumbnailPath($values->asset_id),
    );

    return theme('image_style', $variables);
  }
}
