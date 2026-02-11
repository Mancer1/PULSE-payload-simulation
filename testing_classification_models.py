import uproot
import pandas as pd
import numpy as np
import pickle
import os
from sklearn.metrics import accuracy_score, classification_report

def test_on_new_data(test_root_file, tree_name, model_folder, event_count_tag):
    # 1. LOAD MODEL AND ENCODERS
    model_path = os.path.join(model_folder, f"event_model_{event_count_tag}events.pkl")
    encoder_path = os.path.join(model_folder, f"event_encoders_{event_count_tag}events.pkl")
    
    if not os.path.exists(model_path):
        print(f"Error: Could not find model at {model_path}")
        return

    with open(model_path, "rb") as f:
        model = pickle.load(f)
    with open(encoder_path, "rb") as f:
        encoders = pickle.load(f)
        le_type = encoders['type']
        le_energy = encoders['energy']

    print(f"--- Model Loaded: {model_path} ---")

    # 2. LOAD AND AGGREGATE TEST DATA
    print(f"Loading test data: {test_root_file}")
    with uproot.open(test_root_file) as file:
        df = file[tree_name].arrays(library="pd")
    
    agg_logic = {
        'charge': ['sum', 'mean', 'count', 'std'],
        'pixel_x': ['min', 'max', 'std'],
        'pixel_y': ['min', 'max', 'std'],
        'Incident_particle_type': 'first',
        'Incident_particle_energy': 'first'
    }
    
    event_df = df.groupby('event_idx').agg(agg_logic)
    event_df.columns = [f"{col[0]}_{col[1]}" for col in event_df.columns]
    event_df = event_df.fillna(0)
    event_df['cluster_width'] = event_df['pixel_x_max'] - event_df['pixel_x_min']
    event_df['cluster_height'] = event_df['pixel_y_max'] - event_df['pixel_y_min']

    # 3. PREPROCESSING
    features = ['charge_sum', 'charge_mean', 'charge_count', 'charge_std', 
                'cluster_width', 'cluster_height', 'pixel_x_std', 'pixel_y_std']
    
    X_test = event_df[features].to_numpy().astype(float)
    
    # Convert true labels to numbers for comparison
    y_true_type = le_type.transform(event_df['Incident_particle_type_first'].astype(str))
    y_true_energy = le_energy.transform(event_df['Incident_particle_energy_first'].astype(str))

    # 4. RUN PREDICTIONS
    print(f"Running predictions on {len(event_df)} events...")
    predictions = model.predict(X_test)
    
    pred_type_ids = predictions[:, 0]
    pred_energy_ids = predictions[:, 1]

    # 5. RESULTS AND EVALUATION
    print("\n" + "="*40)
    print("        FINAL TEST RESULTS")
    print("="*40)

    # --- PARTICLE TYPE RESULTS ---
    acc_type = accuracy_score(y_true_type, pred_type_ids)
    print(f"\n[PARTICLE TYPE]")
    print(f"Accuracy: {acc_type*100:.2f}%")
    print(classification_report(y_true_type, pred_type_ids, target_names=le_type.classes_))

    # --- ENERGY RESULTS ---
    acc_energy = accuracy_score(y_true_energy, pred_energy_ids)
    print(f"\n[INCIDENT ENERGY]")
    print(f"Accuracy: {acc_energy*100:.2f}%")
    print(classification_report(y_true_energy, pred_energy_ids, target_names=le_energy.classes_))

    # Visualizing first 5 results
    pred_type_names = le_type.inverse_transform(pred_type_ids)
    pred_energy_names = le_energy.inverse_transform(pred_energy_ids)
    
    print("\nFirst 5 Predicted Pairs (Type, Energy):")
    for i in range(5):
        print(f"  Event {i}: {pred_type_names[i]}, {pred_energy_names[i]} GeV")

if __name__ == "__main__":
    DATA_FOLDER = "automation_roots"
    FILE_NAME = "MergedOutput_1000_10mrad.root"
    FILE_TO_TEST = os.path.join(DATA_FOLDER, FILE_NAME)
    
    # Path to your model folder
    FOLDER = "trained_models_for_classification/beam_divergence_10mrad"
    EVENT_COUNT = 1000 
    
    test_on_new_data(FILE_TO_TEST, "pixelcharge_flattened", FOLDER, EVENT_COUNT)