<?php
/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2012 SURFnet BV (http://www.surfnet.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, you can find it at:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 */

/**
 * @file
 * Filter for collection selection in an exposed form.
 */

class mediamosa_ck_views_filter_collections extends mediamosa_ck_views_filter {

  /**
   * Add input to filter data.
   */
  public function query() {
    parent::_cql_exact();
  }

  /**
   * Provide a simple textfield for equality
   */
  public function value_form(&$form, &$form_state) {
    $form['value'] = array(
      '#type' => 'select',
      '#title' => t($this->definition['title']),
      '#options' => array_merge(array('All' => t('- Any -')), MediaMosaCkConnectorWrapper::get_collections_for_options()),
      '#default_value' => $this->value,
    );
  }
}
