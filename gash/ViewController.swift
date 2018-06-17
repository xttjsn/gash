//
//  ViewController.swift
//  CircuitExecutor
//
//  Created by Xiaoting Tang on 6/16/18.
//  Copyright © 2018 Xiaoting Tang. All rights reserved.
//

import Cocoa
import BigInt

class ViewController: NSViewController {
    
    @IBOutlet weak var m_data1_btn: NSButton!
    @IBOutlet weak var m_data0_btn: NSButton!
    @IBOutlet weak var m_circ_btn: NSButton!
    @IBOutlet weak var m_circ_fname: NSTextField!
    @IBOutlet weak var m_data0_fname: NSTextField!
    @IBOutlet weak var m_data1_fname: NSTextField!
    @IBOutlet weak var m_output: NSTextField!
    @IBOutlet weak var m_bitsize: NSTextField!
    @IBOutlet weak var m_denominator: NSTextField!
    @IBOutlet weak var m_numeritor: NSTextField!
    @IBOutlet weak var m_divres: NSTextField!
    override func viewDidLoad() {
        super.viewDidLoad()
        
        // Do any additional setup after loading the view.
    }
    
    override var representedObject: Any? {
        didSet {
            // Update the view, if already loaded.
        }
    }
    
    @IBAction func browseFile(_ sender: AnyObject) {
        
        let dialog = NSOpenPanel();
        
        dialog.title                   = "Choose a .circ file";
        dialog.showsResizeIndicator    = true;
        dialog.showsHiddenFiles        = false;
        dialog.canChooseDirectories    = true;
        dialog.canCreateDirectories    = true;
        dialog.allowsMultipleSelection = false;
        dialog.allowedFileTypes        = nil;
        
        if (dialog.runModal() == NSApplication.ModalResponse.OK) {
            let result = dialog.url // Pathname of the file
            
            
            if (result != nil) {
                let path = result!.path
                
                if ((sender as! NSButton).tag == m_circ_btn.tag) {
                    m_circ_fname.stringValue = path
                } else if ((sender as! NSButton).tag == m_data0_btn.tag) {
                    m_data0_fname.stringValue = path
                } else if ((sender as! NSButton).tag == m_data1_btn.tag) {
                    m_data1_fname.stringValue = path
                }
            }
        } else {
            // User clicked on "Cancel"
            return
        }
        
    }
    
    
    @IBAction func executeFile(sender: AnyObject) {
        
        let circ = Circuit()
        circ.build_from_file(circ_fpath: m_circ_fname.stringValue)
        circ.read_input(data_fpath: m_data0_fname.stringValue)
        circ.read_input(data_fpath: m_data1_fname.stringValue)
        circ.execute()
        let outstr = circ.get_output_str()
        
        var output = BigInt(outstr, radix: 2)
        let ring = BigInt(String(repeating: "1", count: Int(m_bitsize.stringValue)!), radix: 2)
        
        if (Array(outstr)[0] == "1") {
            output = ring! - output!
            output = -output!
        }
        
        m_output.stringValue = output!.description
    }
    
    @IBAction func calc_div(_ sender: Any) {
        let denom = Int(m_denominator.stringValue, radix: 10)
        let nume = Int(m_numeritor.stringValue, radix: 10)
        
        var circ : OpaquePointer? = nil
        circ = CreateCDivCircuit(CInt(m_bitsize.stringValue)!, CInt(denom!), CInt(nume!))
        BuildDivCircuit(circ)
        ExecuteCircuit(circ)
        let outstr = String(cString: GetOutputString(circ));
        let ring = BigInt(String(repeating: "1", count: Int(m_bitsize.stringValue)!), radix: 2)

        var output = BigInt(outstr, radix: 2)
        if (Array(outstr)[0] == "1") {
            output = ring! - output!
            output = -output! - 1
        }

        m_divres.stringValue = output!.description

    }
    
    
    @IBAction func calc_div_orig(_ sender: Any) {
        let denom = Int(m_denominator.stringValue, radix: 10)
        let nume = Int(m_numeritor.stringValue, radix: 10)
        
        let circ = DivCircuit(bitsize: Int(m_bitsize.stringValue)!)
        circ.set_denom(denom: denom!)
        circ.set_nume(nume: nume!)
        circ.build()
        circ.execute()
        
        let outstr = circ.get_output_str()
        
        var output = BigInt(outstr, radix: 2)
        let ring = BigInt(String(repeating: "1", count: Int(m_bitsize.stringValue)!), radix: 2)
        
        if (Array(outstr)[0] == "1") {
            output = ring! - output!
            output = -output! - 1
        }
        
        m_divres.stringValue = output!.description
    }
    
    
}

