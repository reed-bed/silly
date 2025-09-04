// Start with a copy-paste of one of the Beryllium examples to get started

use std::collections::HashMap;
use std::fs::File;
use std::io::prelude::*;

use rand::prelude::*;

use beryllium::{
    Sdl,
    events::Event,
    init::InitFlags,
    video::{CreateWinArgs, RendererFlags},
};
use core::ffi::c_int;
use pixel_formats::r8g8b8a8_Srgb;

use authorsphere::author_tools::AuthorID;

fn main() {
    // get collaborator network
    let root_id = AuthorID::RefID("1006450".to_string());

    // see if we saved any useful data already
    let mut tab_buffer = String::new();
    let mut tab_file = File::open("authorsphere.ron")
        .map(|mut x| x.read_to_string(&mut tab_buffer))
        .ok();
    let tab: Option<authorsphere::author_tools::AuthorTable> = ron::de::from_str(&tab_buffer).ok();

    let authorsphere = authorsphere::map_collaborators(&root_id, 3, 2019, 10, tab);

    let s = ron::to_string(&authorsphere).unwrap();
    std::fs::write("authorsphere.ron", &s).ok();

    // these will be the points of our visualisation
    let author_list = authorsphere.all_authors();
    let total_authors = author_list.len();
    // println!("{author_list:?}");
    let mut plot_points = HashMap::<&AuthorID, [c_int; 2]>::new();

    let mut rng = rand::rng();

    // Initializes SDL2
    let sdl = Sdl::init(InitFlags::EVERYTHING);

    for info in sdl.get_renderer_driver_infos().unwrap() {
        println!("RendererDriver: {info:?}");
    }

    // Makes the window with an associated SDL Renderer.
    let win = sdl
        .create_renderer_window(
            CreateWinArgs {
                title: "Example Renderer Window",
                width: 1800,
                height: 900,
                ..Default::default()
            },
            RendererFlags::ACCELERATED_VSYNC,
        )
        .unwrap();
    println!("Created The Renderer Window!");
    println!("Selected Renderer Info: {:?}", win.get_renderer_info());

    let pix_buf = [r8g8b8a8_Srgb {
        r: 255,
        g: 127,
        b: 16,
        a: 255,
    }; 64];
    let surface = sdl.create_surface_from(&pix_buf, 8, 8).unwrap(); //let tex = win.create_texture_from_surface(&surface).unwrap();

    // instantiate plot points at random coordinates
    let (width, height) = win.get_window_size();
    let frac = 4;
    for author in author_list.iter() {
        plot_points.insert(
            author,
            [rng.random_range(-width/frac..width/frac) + width/2, rng.random_range(-height/frac..height/frac) + height/2],
        );
    }

    // program "main loop".
    let mut time = 0_f64;
    'the_loop: loop {
        // Process events from this frame.
        #[allow(clippy::never_loop)]
        #[allow(clippy::single_match)]
        while let Some((event, _timestamp)) = sdl.poll_events() {
            match event {
                Event::Quit => break 'the_loop,
                _ => (),
            }
        }

        win.set_draw_color(u8::MAX, u8::MAX, u8::MAX, u8::MAX)
            .unwrap();
        win.clear().unwrap();

        win.set_draw_color(0, 0, 0, u8::MAX).unwrap();

        let points_to_draw = plot_points.values().cloned().collect::<Vec<[c_int; 2]>>();
        win.draw_points(&points_to_draw[..]).unwrap();

        // draw lines for all collaborations
        for auth in author_list.iter() {
            for other in authorsphere.get_author(*auth).unwrap().get_collaborators() {
                let p1 = plot_points.get(auth).unwrap().clone();
                if let Some(p2) = plot_points.get(&other) {
                    let p2 = p2.clone();
                    win.draw_lines(&[p1, p2]).unwrap();
                }
            }
        }

        // do some physics to relax the network. parameters chosen to create
        // sensible spacing at isotropic parts of network
        // assuming a damping-dominated regime.
        let step = 1.0;
        time += step as f64;
        println!("{time}");
        let repel_power = 2.8;
        let softening = (-time as f32 / 1000.0).exp();
        let damping = 1.0 * (1.0 - 0.9 * softening.powf(0.3));
        let r = ((width * width) as f32 + (height * height) as f32).powf(0.5);
        let A = (1.0 - softening) * 0.005 / (1.8 * total_authors as f32).powf(1.5);
        let B = |nu: u32, total_u: u32| {
            let n = nu as f32;
            let total = total_u as f32;
            if nu == 0 {
                0.0
            } else {
                65.0 * n.powf(1.1) / (total.powf(1.1) * (1.0 - 0.5 * softening.powi(2)))
            }
        }; //+ (n/5.0).powf(0.5)}};

        let mut prev_points = plot_points.clone();
        for auth in prev_points.keys() {
            let mut auth_pos = prev_points.get(auth).unwrap().clone();
            let a = authorsphere.get_author(auth).unwrap();

            let cs = a.get_collaborators();

            let total_collaborators = cs.iter().map(|x| plot_points.get(x)).fold(0, |base, c| {
                base + match c {
                    Some(_) => 1,
                    None => 0,
                }
            });
            if total_collaborators == 0 {
                plot_points.remove(auth);
                //panic!("{auth:?}");

            }
            for other in prev_points.keys() {
                if other == auth {
                    continue;
                };
                if let Some(p2) = plot_points.get(other) {
                    let displacement = [p2[0] - auth_pos[0], p2[1] - auth_pos[1]];
                    let d_len = displacement
                        .iter()
                        .fold(0.0, |acc, &el| acc + (el as f32).powi(2))
                        .sqrt();
                    let n_collaborations = authorsphere
                        .get_author(auth)
                        .unwrap()
                        .get_collaboration(&other)
                        .unwrap_or(&0);
                    let B = B(*n_collaborations, total_collaborators);
                    let change = displacement.map(|x| {
                        ((step / damping)
                            * (-x as f32
                                * A * (total_collaborators as f32 * 10.0/ (10.0
                                        + total_collaborators as f32))
                                * (if total_collaborators == 0 {
                                    1.0 - softening
                                } else {
                                    1.0
                                })
                                / ((d_len / r).powf(repel_power))
                                + B * (x as f32 / r) * (d_len / r).powf(0.1)))
                            as i32
                    });
                    auth_pos[0] +=
                        change[0] + (rng.random_range(-0.5..0.5) * softening / damping.sqrt()) as i32;
                    auth_pos[1] +=
                        change[1] + (rng.random_range(-0.5..0.5) * softening / damping.sqrt()) as i32;
                }
                let p0 = auth_pos[0] as f32 - (width/2) as f32;
                let p1 = auth_pos[1] as f32 - (height/2) as f32;
                auth_pos[0] -=(step / damping * 0.001 * p0 * (p0.abs()  /width as f32).powf(1.0))as i32;
                auth_pos[1] -=(step / damping * 0.001 * p1 * (p1.abs() /height as f32).powf(1.0))as i32;
                auth_pos[0] = auth_pos[0].min(width).max(0);
                auth_pos[1] = auth_pos[1].min(height).max(0);
                plot_points.insert(auth, auth_pos);
            }
        }

        // except that the root element should be stuck at the midpoint
        plot_points.insert(&root_id, [width / 2 as c_int, height / 2 as c_int]);

        win.present();
    }

    // All the cleanup is handled by the various drop impls.
}
