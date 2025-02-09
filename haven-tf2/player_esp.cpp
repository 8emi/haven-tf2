#include "sdk.h"
#include "movement_simulate.h"
#include <string.h>
#include "i_client_entity.h"
void c_visuals::player_esp()
{
    for (auto i = 1; i < g_interfaces.m_engine->get_max_clients(); i++)
    {
        auto& player_esp = g_ui.m_controls.visuals.players;

        if (!player_esp.enabled->m_value)
        {
            continue;
        }

            
        const auto player = g_interfaces.m_entity_list->get_entity<c_base_player>(i);

        int alpha = 122.f;
        int low_alpha = 80.f;
        color dorcolors = {255, 255, 255, alpha};
        if (!player->is_valid(g_cl.m_local, true, false, true))
            continue;
        if (player->is_valid(g_cl.m_local, true, true, true))
        {

           alpha = 255.f;
           low_alpha = 179.f;
           dorcolors = {0, 255, 0, alpha};

       
        }
        auto shared = player->m_shared();
        if (!shared)
            continue;

        auto bounding_box = player->get_bounding_box();

        if (bounding_box.is_zero())
        {
            if (!g_ui.m_controls.visuals.players.offscreen->m_value)
                continue;
            vector vec_delta = player->m_vec_origin();
            vec_delta -= g_cl.m_local->m_vec_origin();

            const auto yaw = rad_to_deg(atan2f(vec_delta.m_y, vec_delta.m_x));
            vector angles;
            g_interfaces.m_engine->get_view_angles(angles);
            const auto yaw_delta = deg_to_rad((angles.m_y - yaw) - 90.f);

            const auto clr = color{0x81, 0xFF, 0x21};
            auto r_x = 0.5f, r_y = 0.5f;
            int width, height;
            g_interfaces.m_engine->get_screen_size(width, height);
            r_x *= width / 2.f;
            r_y *= height / 2.f;

            constexpr auto triangle_size = 15.f;

            Vertex_t triag[3] = {
                {{width / 2.f + cos(yaw_delta) * (r_x), height / 2.f + sin(yaw_delta) * (r_y)}, vector_2d(1,1)},
                {{width / 2.f + (cos(yaw_delta) * (r_x - triangle_size)) +
                      (cos(yaw_delta + deg_to_rad(90)) * triangle_size),
                  height / 2.f +
                      (sin(yaw_delta) * (r_y - triangle_size) + (sin(yaw_delta + deg_to_rad(90)) * triangle_size))},
                 vector_2d(1, 1)},
                {{width / 2.f + (cos(yaw_delta) * (r_x - triangle_size)) +
                      (cos(yaw_delta + deg_to_rad(90)) * -triangle_size),
                  height / 2.f +
                      (sin(yaw_delta) * (r_y - triangle_size) + (sin(yaw_delta + deg_to_rad(90)) * -triangle_size))},
                 vector_2d(1, 1)},
            };

            g_render.render_verts(3, triag, g_ui.m_theme);
            continue;
        }

        const auto health = player->m_i_health();
        const auto max_health = player->get_max_health();

        player_info_s player_info;

        if (player_esp.name->m_value)
            if (g_interfaces.m_engine->get_player_info(player->entindex(), &player_info))
                g_render.text(g_render.m_fonts.main,
                              {bounding_box.m_x + (bounding_box.m_w * 0.5f), bounding_box.m_y - 15}, player_info.m_name,
                              g_cl.m_local->m_i_team_num() == player->m_i_team_num() ? g_ui.m_theme
                                                                                     : color(255, 255, 255, alpha),
                              text_align_center);

        if (player_esp.box->m_value)
        {
            g_render.outlined_rect(
                {bounding_box.m_x - 1, bounding_box.m_y - 1, bounding_box.m_w + 2, bounding_box.m_h + 2}, {0, 0, 0, low_alpha});
            g_render.outlined_rect(
                {bounding_box.m_x + 1, bounding_box.m_y + 1, bounding_box.m_w - 2, bounding_box.m_h - 2}, {0, 0, 0, low_alpha});
            g_render.outlined_rect(bounding_box, {255, 255, 255, low_alpha});
        }

        if (player_esp.health->m_selected_index == 1)
        {
            const box_t health_bar_area = {bounding_box.m_x - 6, bounding_box.m_y, 4, bounding_box.m_h};

            // get our healthbar height.
            const auto healthbar_height = health_bar_area.m_h * math::get_fraction(health, max_health, 0);

            g_render.filled_rect(health_bar_area, {35, 35, 35, 255});
            g_render.filled_rect({health_bar_area.m_x, health_bar_area.m_y + (health_bar_area.m_h - healthbar_height),
                                  health_bar_area.m_w, healthbar_height},
                                 dorcolors);
            g_render.outlined_rect(health_bar_area, {0, 0, 0, 255});



            char buffer[128];
            _itoa(health, buffer, 10);
            if (health < max_health)
            {
                g_render.text(g_render.m_fonts.secondary,
                              {bounding_box.m_x - 2, bounding_box.m_y + health_bar_area.m_h - healthbar_height - 1},
                              buffer, color(255, 255, 255, 175), text_align_right);
            }
         
        }
        else if (player_esp.health->m_selected_index == 2)
        {
            char buffer[128];
            _itoa(health, buffer, 10);

            g_render.text(g_render.m_fonts.secondary, {bounding_box.m_x - 2, bounding_box.m_y}, buffer,
                          color(255, 255, 255), text_align_right);
        }

        // eventually do multi combo box.
        if (player_esp.flags->m_value)
        {
            float flag_offset = 0.f;
            auto draw_flag =
                [&flag_offset, &bounding_box, player](const char* flag_text, const color& flag_color) mutable
            {
                const color col = {255, 255, 255};

                const vector_2d flag_text_size = g_render.get_text_size(flag_text, g_render.m_fonts.secondary);
                g_render.text(g_render.m_fonts.secondary,
                              {(bounding_box.m_x + bounding_box.m_w) + 4, (bounding_box.m_y + flag_offset) - 2},
                              flag_text, col);

                flag_offset += flag_text_size.m_y + 1;
            };
         
            if (health > max_health)
                draw_flag("OH", color(255, 255, 255, alpha));

            if (health < max_health)
                draw_flag("HIT", color(255, 255, 255, alpha));

             if (player->is_dormant())
               draw_flag("DORMANT", color(255, 255, 255, alpha));

            if (shared->in_cond(e_tf_cond::TF_COND_DISGUISED))
                draw_flag("DG", color(255, 255, 255, alpha));

            if (shared->in_cond(e_tf_cond::TF_COND_STEALTHED))
                draw_flag("CLOAK", color(255, 255, 255,alpha));

            if (shared->in_cond(e_tf_cond::TF_COND_PHASE))
                draw_flag("BONK", color(255, 255, 255, alpha));

            if (shared->in_cond(e_tf_cond::TF_COND_INVULNERABLE))
                draw_flag("CRIT", color(255, 255, 255, alpha));

            if (shared->in_cond(e_tf_cond::TF_COND_TAUNTING))
                draw_flag("TAUNT", color(255, 255, 255, alpha));

            if (shared->in_cond(e_tf_cond::TF_COND_ZOOMED))
                draw_flag("ZOOM", color(60, 20, 225, alpha));
          //idk if im stupid or something but the color doesnt seem to change from white
            if (shared->in_cond(e_tf_cond::TF_COND_BLEEDING))
                draw_flag("BLEED", color(255, 5, 5, alpha));
        }

        if (player_esp.weapon->m_value)
        {
            const auto weapon = reinterpret_cast<c_base_weapon*>(player->get_active_weapon());
            if (!weapon)
            {
                return;
            }

            std::string weapon_text = weapon->get_localized_name();

            // smallfonts needs upper case.
            std::transform(weapon_text.begin(), weapon_text.end(), weapon_text.begin(), ::toupper);
            g_render.text(g_render.m_fonts.secondary,
                          {bounding_box.m_x + (bounding_box.m_w * 0.5f), bounding_box.m_y + bounding_box.m_h},
                           weapon_text.c_str(),
                          {255, 255, 255, alpha}, text_align_center);
        }
    }

    // predicted path
    if (g_cl.m_local->is_valid(g_cl.m_local, false))
    {
        vector last_pos = g_cl.m_local->m_vec_origin();
        vector screen_1;
        vector screen_2;

        if (!g_movement.setup_mv(g_cl.m_local->m_velocity(), g_cl.m_local))
            return;

        for (auto i = 0; i < 100; i++)
        {
            vector nul;
            if (!g_movement.run(nul))
                break;
        }
        g_movement.m_logs[g_cl.m_local->entindex()].m_path = g_movement.path;
        g_movement.m_logs[g_cl.m_local->entindex()].end_time = g_movement.end_time;
    }
}
