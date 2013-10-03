<?php
/**
 * @file
 * The plugin query object for MediaMosa allows REST calls inside views. It
 * uses the Views 3 plugin option to query another source instead of the normal
 * Drupal database.
 */

abstract class mediamosa_ck_views_rest_asset extends mediamosa_ck_views_rest {

  // ------------------------------------------------------------------ Methods.
  /**
   * Provide a form to edit options for this plugin.
   */
  public function options_form(&$form, &$form_state) {
    // Call parent.
    parent::options_form($form, $form_state);

    // Checkbox to include still information.
    $form['mediamosa_ck_views_show_stills'] = array(
      '#title' => t('Include still information in the view.'),
      '#description' => t('This switch will enable still information to be available. The view response will be a bit faster when off. Switch is enabled by default.'),
      '#type' => 'checkbox',
      '#default_value' => $this->mediamosa_option_show_stills(),
    );

    // Checkbox to include the collection information.
    $form['mediamosa_ck_views_show_collections'] = array(
      '#title' => t('Include collection information in the view.'),
      '#description' => t('This switch will enable collection information to be available. The view response will be a bit faster when off. Switch is disabled by default.'),
      '#type' => 'checkbox',
      '#default_value' => $this->mediamosa_option_show_collections(),
    );
  }

  /**
   * Include the stills in the view output.
   *
   * @return boolean
   *   The value of mediamosa_show_stills.
   */
  public function mediamosa_option_show_stills() {

    if (isset($this->options['mediamosa_ck_views_show_stills'])) {
      return $this->options['mediamosa_ck_views_show_stills'];
    }

    // Return the show stills value,
    return TRUE;
  }

  /**
   * Include the collections in the view output.
   *
   * @return boolean
   *   The value of mediamosa_show_collections.
   */
  public function mediamosa_option_show_collections() {

    if (isset($this->options['mediamosa_ck_views_show_collections'])) {
      return $this->options['mediamosa_ck_views_show_collections'];
    }

    // Return the show collections info.
    return FALSE;
  }
}
