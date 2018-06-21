//
//  ViewController.swift
//  CircuitExecutor
//
//  Created by Xiaoting Tang on 6/16/18.
//  Copyright © 2018 Xiaoting Tang. All rights reserved.
//

import Cocoa
import BigInt

class ViewController: NSViewController, NSComboBoxDelegate, NSTextViewDelegate {
    
    @IBOutlet weak var m_btn_data1: NSButton!
    @IBOutlet weak var m_btn_data0: NSButton!
    @IBOutlet weak var m_btn_circ: NSButton!
    @IBOutlet weak var m_circ_fname: NSTextField!
    @IBOutlet weak var m_data0_fname: NSTextField!
    @IBOutlet weak var m_data1_fname: NSTextField!
    @IBOutlet weak var m_txt_output: NSTextField!
    @IBOutlet weak var m_txt_bitsize: NSTextField!
    @IBOutlet weak var m_txt_denominator: NSTextField!
    @IBOutlet weak var m_txt_numerator: NSTextField!
    @IBOutlet weak var m_txt_div_result: NSTextField!
    @IBOutlet weak var m_combo_circ: NSComboBox!
    @IBOutlet weak var m_txt_garbler_raw_output: NSTextField!
    @IBOutlet weak var m_txt_evaluator_raw_output: NSTextField!
    @IBOutlet weak var m_txt_garbler_output: NSTextField!
    @IBOutlet weak var m_txt_evaluator_output: NSTextField!
    @IBOutlet weak var m_txt_garbler_ip: NSTextField!
    @IBOutlet weak var m_txt_evaluator_ip: NSTextField!
    @IBOutlet weak var m_img_garbler_status: NSImageView!
    @IBOutlet weak var m_img_evaluator_status: NSImageView!
    @IBOutlet var m_txt_circ: NSTextView!
    
    var m_circ_name : String!
    var m_bitsize   : Int!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        let items = ["billionaire", "add", "sub", "div", "mul", "relu"]
        m_combo_circ.removeAllItems()
        m_combo_circ.addItems(withObjectValues: items);
        m_combo_circ.selectItem(at: 0)
        
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
                
                if ((sender as! NSButton).tag == m_btn_circ.tag) {
                    m_circ_fname.stringValue = path
                } else if ((sender as! NSButton).tag == m_btn_data0.tag) {
                    m_data0_fname.stringValue = path
                } else if ((sender as! NSButton).tag == m_btn_data1.tag) {
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
        let ring = BigInt(String(repeating: "1", count: Int(m_txt_bitsize.stringValue)!), radix: 2)
        
        if (Array(outstr)[0] == "1") {
            output = ring! - output!
            output = -output!
        }
        
        m_txt_output.stringValue = output!.description
    }
    
    @IBAction func calc_div(_ sender: Any) {
        let denom = Int(m_txt_denominator.stringValue, radix: 10)
        let nume = Int(m_txt_numerator.stringValue, radix: 10)
        
        var circ : OpaquePointer? = nil
        circ = CreateCDivCircuit(CInt(m_bitsize), CInt(denom!), CInt(nume!))
        BuildDivCircuit(circ)
        ExecuteCircuit(circ)
        let outstr = String(cString: GetOutputString(circ));
        let ring = BigInt(String(repeating: "1", count: m_bitsize ), radix: 2)

        var output = BigInt(outstr, radix: 2)
        if (Array(outstr)[0] == "1") {
            output = ring! - output!
            output = -output! - 1
        }

        m_txt_div_result.stringValue = output!.description

    }
    
    func recover_binary_output(outstr: String) -> String {
        let ring = BigInt(String(repeating: "1", count: m_bitsize), radix: 2)
        var output = BigInt(outstr, radix: 2)
        if (Array(outstr)[0] == "1") {
            output = ring! - output!
            output = -output! - 1
        }
        return output!.description
    }
    
    @IBAction func calc_div_orig(_ sender: Any) {
        let denom = Int(m_txt_denominator.stringValue, radix: 10)
        let nume = Int(m_txt_numerator.stringValue, radix: 10)
        
        let circ = DivCircuit(bitsize: Int(m_txt_bitsize.stringValue)!)
        circ.set_denom(denom: denom!)
        circ.set_nume(nume: nume!)
        circ.build()
        circ.execute()
        
        let outstr = circ.get_output_str()
        m_txt_div_result.stringValue = recover_binary_output(outstr: outstr)
    }
    
    
    @IBAction func garbler_start(_ sender: Any) {
        m_img_garbler_status.image = #imageLiteral(resourceName: "Yellow")
        
        SetCircuitFunc(m_txt_circ.textStorage?.mutableString.cString(using: String.Encoding(rawValue: String.Encoding.ascii.rawValue).rawValue))
        StartGarbler(m_txt_evaluator_ip.stringValue.cString(using: String.Encoding(rawValue: String.Encoding.ascii.rawValue)))
        m_txt_garbler_raw_output.stringValue = String(cString: GetGarblerRawOutput())
        ResetGarbler();
        m_txt_garbler_output.stringValue = recover_binary_output(outstr: m_txt_garbler_raw_output.stringValue)
        
        m_img_garbler_status.image = #imageLiteral(resourceName: "GreenLight")
    }
    
    @IBAction func evaluator_start(_ sender: Any) {
        m_img_evaluator_status.image = #imageLiteral(resourceName: "Yellow")
        
        SetCircuitFunc(m_txt_circ.textStorage?.mutableString.cString(using: String.Encoding(rawValue: String.Encoding.ascii.rawValue).rawValue))
        StartEvaluator(m_txt_garbler_ip.stringValue.cString(using: String.Encoding(rawValue: String.Encoding.ascii.rawValue)))
        m_txt_evaluator_raw_output.stringValue = String(cString: GetEvaluatorRawOutput())
        ResetEvaluator();
        m_txt_evaluator_output.stringValue = recover_binary_output(outstr: m_txt_evaluator_raw_output.stringValue)
        
        m_img_evaluator_status.image = #imageLiteral(resourceName: "GreenLight")
    }
    
    func set_circuit_name(circname: String) {
        m_circ_name = circname
    }
    
    func set_bitsize(bitsize: Int?) {
        if bitsize == nil {
            m_bitsize = 64  // Default value
        } else {
            m_bitsize = bitsize
        }
        
        if m_circ_name != nil {
            load_circuit()
        }
    }
    
    func load_circuit() {
        let m_circ_str = String(cString: LoadCircuitFunc(m_circ_name.cString(using: String.Encoding(rawValue: String.Encoding.ascii.rawValue)), Int32(m_bitsize)))
        m_txt_circ.textStorage?.mutableString.setString(m_circ_str)
    }
    
    func comboBoxSelectionDidChange(_ notification: Notification) {
        let val = m_combo_circ.objectValueOfSelectedItem as! String
        set_bitsize(bitsize: Int(m_txt_bitsize.stringValue))
        set_circuit_name(circname: val)
        load_circuit()
    }
    
    @IBAction func bitsize_change(_ sender: Any) {
        set_bitsize(bitsize: Int(m_txt_bitsize.stringValue))
    }
    
    @IBAction func cross_check(_ sender: Any) {
        SetCircuitFunc(m_txt_circ.textStorage?.mutableString.cString(using: String.Encoding(rawValue: String.Encoding.ascii.rawValue).rawValue))
        CrossCheck();
    }
    
}

