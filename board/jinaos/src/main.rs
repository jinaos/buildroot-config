use evdev::{Device, InputEventKind, EventType, Key, InputEvent};
use nix::unistd::{fork, ForkResult};
use nix::sys::wait::wait;
use std::fs;
use std::io;
use std::process::Command;
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::Arc;
use std::time::{Instant, Duration};

const EDGE_THRESHOLD: i32 = 50;
const SWIPE_MIN_DISTANCE: i32 = 100;
const SWIPE_MAX_TIME: Duration = Duration::from_millis(500);

#[derive(Debug)]
struct TouchPoint {
    x: i32,
    y: i32,
    timestamp: Instant,
}

#[derive(Debug, PartialEq)]
enum Gesture {
    Back,
    Forward,
    Home,
    Recents,
    ControlCenter,
    None,
}

fn find_touchscreen() -> Option<Device> {
    for i in 0..32 {
        let path = format!("/dev/input/event{}", i);
        if let Ok(device) = Device::open(&path) {
            let supported = device.supported_events().unwrap_or_default();
            if supported.event_types.contains(EventType::ABSOLUTE) {
                return Some(device);
            }
        }
    }
    None
}

fn detect_gesture(start: &TouchPoint, end: &TouchPoint, edge: i32) -> Gesture {
    let dx = end.x - start.x;
    let dy = end.y - start.y;
    let distance = (dx*dx + dy*dy) as f64;
    
    if distance < (SWIPE_MIN_DISTANCE * SWIPE_MIN_DISTANCE) as f64 {
        return Gesture::None;
    }
    
    match edge {
        1 => { // Left edge
            if dy.abs() < dx.abs() && dx > 0 {
                Gesture::Forward
            } else {
                Gesture::None
            }
        },
        2 => { // Right edge
            if dy.abs() < dx.abs() && dx < 0 {
                Gesture::Back
            } else {
                Gesture::None
            }
        },
        3 => { // Top edge
            if dx.abs() < dy.abs() && dy > 0 {
                Gesture::ControlCenter
            } else {
                Gesture::None
            }
        },
        4 => { // Bottom edge
            if dx.abs() < dy.abs() && dy < 0 {
                if end.timestamp - start.timestamp < Duration::from_millis(300) {
                    Gesture::Home
                } else {
                    Gesture::Recents
                }
            } else {
                Gesture::None
            }
        },
        _ => Gesture::None,
    }
}

fn execute_gesture(gesture: Gesture) {
    let cmd = match gesture {
        Gesture::Back => "xdotool key Alt+Left",
        Gesture::Forward => "xdotool key Alt+Right", 
        Gesture::Home => "xdotool key Super+d",
        Gesture::Recents => "xdotool key Alt+Tab",
        Gesture::ControlCenter => "swaync-client -t",
        Gesture::None => return,
    };
    
    match unsafe { fork() } {
        Ok(ForkResult::Child) => {
            Command::new("sh")
                .arg("-c")
                .arg(cmd)
                .status()
                .ok();
            std::process::exit(0);
        },
        Ok(ForkResult::Parent { child }) => {
            wait().ok();
        },
        Err(_) => eprintln!("Failed to fork"),
    }
}

fn is_within_edge(x: i32, y: i32, width: i32, height: i32) -> i32 {
    if x < EDGE_THRESHOLD { return 1; }
    if x > width - EDGE_THRESHOLD { return 2; }
    if y < EDGE_THRESHOLD { return 3; }
    if y > height - EDGE_THRESHOLD { return 4; }
    0
}

fn main() -> io::Result<()> {
    println!("JinaOS Gesture Daemon starting...");
    
    let device = find_touchscreen().expect("No touchscreen found");
    let width = 720;
    let height = 1280;
    
    let mut touches = std::collections::HashMap::new();
    let running = Arc::new(AtomicBool::new(true));
    
    let r = running.clone();
    ctrlc::set_handler(move || {
        r.store(false, Ordering::SeqCst);
    }).expect("Error setting Ctrl-C handler");
    
    let mut events = device.into_events();
    
    while running.load(Ordering::SeqCst) {
        if let Ok(event) = events.next_event() {
            match event.kind() {
                InputEventKind::AbsAxis(axis) => {
                    let value = event.value();
                    if let Some(slot) = event.raw_code() {
                        let touch = touches.entry(slot).or_insert_with(|| TouchPoint {
                            x: 0, y: 0, timestamp: Instant::now(),
                        });
                        
                        if axis.0 == 0x35 { // ABS_MT_POSITION_X
                            touch.x = value;
                        } else if axis.0 == 0x36 { // ABS_MT_POSITION_Y
                            touch.y = value;
                        }
                    }
                },
                InputEventKind::Key(key) if key == Key::BTN_TOUCH && event.value() == 0 => {
                    for (slot, touch) in touches.iter() {
                        let edge = is_within_edge(touch.x, touch.y, width, height);
                        if edge != 0 {
                            let gesture = detect_gesture(touch, &TouchPoint {
                                x: 0, y: 0, timestamp: Instant::now(),
                            }, edge);
                            execute_gesture(gesture);
                        }
                    }
                    touches.clear();
                },
                _ => {},
            }
        }
    }
    
    println!("JinaOS Gesture Daemon stopped");
    Ok(())
}