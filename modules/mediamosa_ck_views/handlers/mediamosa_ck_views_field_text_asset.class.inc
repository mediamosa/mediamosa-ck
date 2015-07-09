<?php
/**
 * @file
 * Views field text.
 */

class mediamosa_ck_views_field_text_asset extends views_handler_field {
  /**
   * Get the value that's supposed to be rendered.
   *
   * @param object $values
   *   An object containing all retrieved values.
   * @param string $field
   *   Optional name of the field where the value is stored.
   */
  public function get_value($values, $field = NULL) {
    return $values->{'asset.published'};
  }

  /**
   * Implements render().
   */
  public function render($values) {
    $value = $this->get_value($values);
    return $value;
  }
}
