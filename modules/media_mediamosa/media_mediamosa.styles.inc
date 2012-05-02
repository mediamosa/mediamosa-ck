<?php
/**
 * MediaMosa is a Full Featured, Webservice Oriented Media Management and
 * Distribution platform (http://mediamosa.org)
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
 * Our styles related hoods and functions.
 */

/**
 * Implements hook_styles_register().
 */
function media_mediamosa_styles_register() {
  return array(
    'MediaMediaMosaFileStyles' => array(
      'field_types' => 'file',
      'name' => t('MediaMediaMosa'),
      'description' => t('Media MediaMosa styles.'),
      'path' => drupal_get_path('module', 'media_mediamosa'),
      'file' => 'media_mediamosa.styles.class.inc',
    ),
  );
}

function media_mediamosa_formatter_filter($variables) {
  if (isset($variables['object'])) {
    $object = isset($variables['object']->file) ? $variables['object']->file : $variables['object'];
    return file_uri_scheme($object->uri) == 'mediamosa';
  }
}

/**
 * Implementation of the File Styles module's hook_file_styles_filter().
 */
function media_mediamosa_file_styles_filter($object) {
  $file = isset($object->file) ? $object->file : $object;
  if (file_uri_scheme($file->uri) == 'mediamosa') {
    return 'media_mediamosa';
  }
}

/**
 *  Implements hook_styles_styles().
 */
function media_mediamosa_styles_styles() {
  $styles = array(
    'file' => array(
      'containers' => array(
        'media_mediamosa' => array(
          'styles' => array(
            'mediamosa_thumbnail' => array(
              'name' => 'mediamosa_thumbnail',
              'effects' => array(
                array('label' => t('Thumbnail'), 'name' => 'thumbnail', 'data' => array('thumbnail' => 1)),
                array('label' => t('Resize'), 'name' => 'resize', 'data' => array('width' => 100, 'height' => 75)),
              ),
            ),
            'mediamosa_preview' => array(
              'name' => 'mediamosa_preview',
              'effects' => array(
                array('label' => t('Autoplay'), 'name' => 'autoplay', 'data' => array('autoplay' => 0)),
                array('label' => t('Resize'), 'name' => 'resize', 'data' => array('width' => 220, 'height' => 165)),
              ),
            ),
            'mediamosa_full' => array(
              'name' => 'mediamosa_full',
              'effects' => array(
                array('label' => t('Autoplay'), 'name' => 'autoplay', 'data' => array('autoplay' => 0)),
                array('label' => t('Resize'), 'name' => 'resize', 'data' => array('width' => 640, 'height' => 480)),
                array('label' => t('Full screen'), 'name' => 'fullscreen', 'data' => array('fullscreen' => 1)),
              ),
            ),
          ),
        ),
      ),
    ),
  );

  // Allow any image style to be applied to the thumbnail.
  foreach (image_styles() as $style_name => $image_style) {
    $styles['file']['containers']['media_mediamosa']['styles']['mediamosa_thumbnail_' . $style_name] = array(
      'name' => 'mediamosa_thumbnail_' . $style_name,
      'image_style' => $style_name,
      'effects' => array(
        array('label' => t('Thumbnail'), 'name' => 'thumbnail', 'data' => array('thumbnail' => 1)),
      ),
    );
  }

  return $styles;
}

/**
 *  Implements hook_styles_presets().
 */
function media_mediamosa_styles_presets() {
  $presets = array(
    'file' => array(
      'square_thumbnail' => array(
        'media_mediamosa' => array(
          'mediamosa_thumbnail_square_thumbnail',
        ),
      ),
      'thumbnail' => array(
        'media_mediamosa' => array(
          'mediamosa_thumbnail',
        ),
      ),
      'small' => array(
        'media_mediamosa' => array(
          'mediamosa_preview',
        ),
      ),
      'large' => array(
        'media_mediamosa' => array(
          'mediamosa_full',
        ),
      ),
      'original' => array(
        'media_mediamosa' => array(
          'mediamosa_full',
        ),
      ),
    ),
  );
  return $presets;
}

/**
 * Implements hook_styles_default_containers().
 */
function media_mediamosa_styles_default_containers() {
  // We append MediaMosa to the file containers.
  return array(
    'file' => array(
      'containers' => array(
        'media_mediamosa' => array(
          'class' => 'MediaMediaMosaFileStyles',
          'name' => 'media_mediamosa',
          'label' => t('MediaMosa'),
          'preview' => 'media_mediamosa_preview_style',
        ),
      ),
    ),
  );
}

/**
 * Implements hook_styles_default_presets().
 */
function media_mediamosa_styles_default_presets() {
//  return array(
//    'file' => array(
//      'containers' => array(
//        'media_mediamosa' => array(
//          'default preset' => 'linked_thumbnail',
//          'styles' => array(
//            'original' => array(
//              'default preset' => 'video',
//            ),
//            'thumbnail' => array(
//              'default preset' => 'linked_thumbnail',
//            ),
//            'square_thumbnail' => array(
//              'default preset' => 'linked_square_thumbnail',
//            ),
//            'medium' => array(
//              'default preset' => 'linked_medium',
//            ),
//            'large' => array(
//              'default preset' => 'large_video',
//            ),
//          ),
//          'presets' => array(
//            'unlinked_thumbnail' => array(
//              array(
//                'name' => 'thumbnail',
//                'settings' => array(),
//              ),
//            ),
//            'linked_thumbnail' => array(
//              array(
//                'name' => 'link_to_media',
//                'settings' => array(),
//              ),
//              array(
//                'name' => 'thumbnail',
//                'settings' => array(),
//              ),
//            ),
//            'linked_square_thumbnail' => array(
//              array(
//                'name' => 'link_to_media',
//                'settings' => array(),
//              ),
//              array(
//                'name' => 'image_style',
//                'settings' => array(
//                  'image_style' => 'square_thumbnail',
//                ),
//              ),
//              array(
//                'name' => 'thumbnail',
//                'settings' => array(),
//              ),
//            ),
//            'linked_medium' => array(
//              array(
//                'name' => 'link_to_media',
//                'settings' => array(),
//              ),
//              array(
//                'name' => 'image_style',
//                'settings' => array(
//                  'image_style' => 'medium',
//                ),
//              ),
//              array(
//                'name' => 'thumbnail',
//                'settings' => array(),
//              ),
//            ),
//            'video' => array(
//              array(
//                'name' => 'video',
//                'settings' => array(),
//              ),
//            ),
//            'large_video' => array(
//              array(
//                'name' => 'resize',
//                'settings' => array(
//                  'width' => 480,
//                  'height' => 360,
//                ),
//              ),
//              array(
//                'name' => 'video',
//                'settings' => array(),
//              ),
//            ),
//          ),
//        ),
//      ),
//    ),
//  );
  $presets = array(
    'file' => array(
      'containers' => array(
        'media_mediamosa' => array(
          'default preset' => 'unlinked_thumbnail',
          'styles' => array(
            'original' => array(
              'default preset' => 'video',
            ),
            'thumbnail' => array(
              'default preset' => 'linked_thumbnail',
            ),
            'square_thumbnail' => array(
              'default preset' => 'linked_square_thumbnail',
            ),
            'medium' => array(
              'default preset' => 'linked_medium',
            ),
            'large' => array(
              'default preset' => 'large_video',
            ),
          ),
          'presets' => array(
            'video' => array(
              array(
                'name' => 'video',
                'settings' => array(),
              ),
            ),
            'large_video' => array(
              array(
                'name' => 'resize',
                'settings' => array(
                  'width' => 480,
                  'height' => 360,
                ),
              ),
              array(
                'name' => 'video',
                'settings' => array(),
              ),
            ),
          ),
        ),
      ),
    ),
  );
  // Allow any image style to be applied to the thumbnail.
  foreach (image_styles() as $style_name => $image_style) {
    $presets['file']['containers']['media_mediamosa']['presets']['linked_' . $style_name] = array(
      array(
        'name' => 'linkToMedia',
        'settings' => array(),
      ),
      array(
        'name' => 'imageStyle',
        'settings' => array(
          'image_style' => $style_name,
        ),
      ),
      array(
        'name' => 'thumbnail',
        'settings' => array(),
      ),
    );
    $presets['file']['containers']['media_mediamosa']['presets']['unlinked_' . $style_name] = $presets['file']['containers']['media_mediamosa']['presets']['linked_' . $style_name];
    array_shift($presets['file']['containers']['media_mediamosa']['presets']['unlinked_' . $style_name]);
    foreach ($image_style['effects'] as $effect) {
      if (in_array($effect['name'], array('image_scale', 'image_scale_and_crop', 'image_resize', 'image_crop'))) {
        $presets['file']['containers']['media_mediamosa']['presets']['video_' . $style_name] = array(
          array(
            'name' => 'resize',
            'settings' => $effect['data'],
          ),
          array(
            'name' => 'video',
            'settings' => array(),
          ),
        );
      }
    }
  }
  return $presets;
}
